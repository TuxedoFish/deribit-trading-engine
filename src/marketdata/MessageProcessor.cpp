#include "../../include/marketdata/MessageProcessor.h"

MessageProcessor::MessageProcessor(SBEBinaryWriter& writer)
    : m_writer(writer), securityIdCounter(0)
{
    // Initialize state
    securitiesInfo.assign(100, SecurityInfo{});
}

void MessageProcessor::onMessage(const FIX44::MarketDataSnapshotFullRefresh& message, const FIX::SessionID& sessionID)
{
    const uint64_t timestamp = GetSendingTime(static_cast<FIX44::Message>(message));

    const std::string& symbol = message.getField(FIX::FIELD::Symbol);
    auto it = m_symbolToSecurityId.find(symbol);
    if (it == m_symbolToSecurityId.end())
    {
        std::cerr << "No matching security found for " << symbol << std::endl;
        return;
    }
    int securityId = it->second;

    // Check if this is a trade snapshot by checking first entry (all entries are same type)
    FIX::NoMDEntries noMDEntriesField;
    message.get(noMDEntriesField);
    int noMDEntries = noMDEntriesField.getValue();

    if (noMDEntries == 0)
    {
        // No entries
        return;
    }

    if (noMDEntries > 0)
    {
        FIX44::MarketDataSnapshotFullRefresh::NoMDEntries firstEntry;
        message.getGroup(1, firstEntry);

        FIX::MDEntryType mdEntryType;
        firstEntry.get(mdEntryType);

        if (mdEntryType.getValue() == FIX::MDEntryType_TRADE)
        {
            // This is a trade snapshot, ignore it entirely
            return;
        }
    }

    // Prepare MDFullBook message
    if (!m_writer.prepareMessage(m_mdFullBook))
    {
        std::cerr << "Error preparing MDFullBook message" << std::endl;
        return;
    }

    m_mdFullBook.securityId(securityId);
    m_mdFullBook.timestamp(timestamp);

    // First count bid/ask entries
    int bidCount = 0;
    int askCount = 0;
    for (int i = 1; i <= noMDEntries; i++)
    {
        FIX44::MarketDataSnapshotFullRefresh::NoMDEntries entry;
        message.getGroup(i, entry);

        FIX::MDEntryType mdEntryType;
        entry.get(mdEntryType);

        mdEntryType.getValue() == FIX::MDEntryType_BID ? bidCount++ : 0;
        mdEntryType.getValue() == FIX::MDEntryType_OFFER ? askCount++ : 0;
    }

    // Process bid entries first (up to MAX_LEVELS)
    auto nBidLevels = std::min(MAX_LEVELS, bidCount);
    auto bidLevels = m_mdFullBook.bidLevelsCount(nBidLevels);
    auto bidIdx = 0;
    auto processedBidIdx = 0;

    for (int i = 1; i <= noMDEntries; i++)
    {
        FIX44::MarketDataSnapshotFullRefresh::NoMDEntries entry;
        message.getGroup(i, entry);

        FIX::MDEntryType mdEntryType;
        entry.get(mdEntryType);

        if (mdEntryType == FIX::MDEntryType_BID)
        {
            if (bidIdx < nBidLevels)
            {
                // Include in full book (first MAX_LEVELS)
                auto bidLevel = bidLevels.next();
                SBEUtils::setPrice(bidLevel.price(), entry.getField(FIX::FIELD::MDEntryPx));
                SBEUtils::setQty(bidLevel.qty(), entry.getField(FIX::FIELD::MDEntrySize));
                bidIdx++;
            }
            processedBidIdx++;
        }
    }

    // Process ask entries second (up to MAX_LEVELS)
    auto nAskLevels = std::min(MAX_LEVELS, askCount);
    auto askLevels = m_mdFullBook.askLevelsCount(nAskLevels);
    auto askIdx = 0;
    auto processedAskIdx = 0;

    for (int i = 1; i <= noMDEntries; i++)
    {
        FIX44::MarketDataSnapshotFullRefresh::NoMDEntries entry;
        message.getGroup(i, entry);

        FIX::MDEntryType mdEntryType;
        entry.get(mdEntryType);

        if (mdEntryType == FIX::MDEntryType_OFFER)
        {
            if (askIdx < nAskLevels)
            {
                // Include in full book (first MAX_LEVELS)
                auto askLevel = askLevels.next();
                SBEUtils::setPrice(askLevel.price(), entry.getField(FIX::FIELD::MDEntryPx));
                SBEUtils::setQty(askLevel.qty(), entry.getField(FIX::FIELD::MDEntrySize));
                askIdx++;
            }
            processedAskIdx++;
        }
    }

    // Write the message
    if (!m_writer.writeMessage(m_mdFullBook))
    {
        std::cerr << "Error writing MDFullBook message" << std::endl;
        return;
    }

    // Process overflow entries as incremental updates
    if (bidCount > MAX_LEVELS || askCount > MAX_LEVELS)
    {
        auto overflowBidIdx = 0;
        auto overflowAskIdx = 0;

        for (int i = 1; i <= noMDEntries; i++)
        {
            FIX44::MarketDataSnapshotFullRefresh::NoMDEntries entry;
            message.getGroup(i, entry);

            FIX::MDEntryType mdEntryType;
            entry.get(mdEntryType);

            if (mdEntryType == FIX::MDEntryType_BID)
            {
                overflowBidIdx++;
                if (overflowBidIdx > MAX_LEVELS)
                {
                    // Process overflow bid entry as incremental update
                    ProcessMDEntry(entry, securityId, timestamp);
                }
            }
            else if (mdEntryType == FIX::MDEntryType_OFFER)
            {
                overflowAskIdx++;
                if (overflowAskIdx > MAX_LEVELS)
                {
                    // Process overflow ask entry as incremental update
                    ProcessMDEntry(entry, securityId, timestamp);
                }
            }
            else if (mdEntryType == FIX::MDEntryType_TRADE)
            {
                // Process trade entries as incremental updates
                ProcessMDEntry(entry, securityId, timestamp);
            }
        }
    }

    // Send out SecurityStatus - Online afterwards
    UpdateSecurityStatus(securityId, timestamp, com::liversedge::messages::SecurityStatusEnum::Value::ONLINE);
}

void MessageProcessor::onMessage(const FIX44::MarketDataIncrementalRefresh& message, const FIX::SessionID& sessionID)
{
    const uint64_t timestamp = GetSendingTime(static_cast<FIX44::Message>(message));

    // Get symbol and find security ID using hash map lookup
    const std::string& symbol = message.getField(FIX::FIELD::Symbol);
    const auto it = m_symbolToSecurityId.find(symbol);
    if (it == m_symbolToSecurityId.end())
    {
        std::cerr << "No matching security found for incremental update: " << symbol << std::endl;
        return;
    }
    const int securityId = it->second;

    // Check security status is ONLINE
    if (securitiesInfo[securityId].status != com::liversedge::messages::SecurityStatusEnum::Value::ONLINE)
    {
        std::cerr << "Ignoring incremental update for offline security: " << symbol << std::endl;
        return;
    }

    // Get number of MD entries
    FIX::NoMDEntries noMDEntriesField;
    message.get(noMDEntriesField);
    const int noMDEntries = noMDEntriesField.getValue();

    if (noMDEntries == 0)
    {
        return;
    }

    // Fast path for single-entry messages (most common case)
    if (noMDEntries == 1)
    {
        FIX44::MarketDataIncrementalRefresh::NoMDEntries entry;
        message.getGroup(1, entry);
        ProcessMDEntry(entry, securityId, timestamp);
        return;
    }

    // Process each MD entry using shared logic
    for (int i = 1; i <= noMDEntries; i++)
    {
        FIX44::MarketDataIncrementalRefresh::NoMDEntries entry;
        message.getGroup(i, entry);
        ProcessMDEntry(entry, securityId, timestamp);
    }
}

void MessageProcessor::onMessage(const FIX44::SecurityList& message, const FIX::SessionID& sessionID)
{
    const uint64_t timestamp = GetSendingTime(static_cast<FIX44::Message>(message));

    // Message contains multiple entries for each security
    FIX::NoRelatedSym noSecuritiesField;
    message.get(noSecuritiesField);
    int noSecurities = noSecuritiesField.getValue();

    // FIX repeating groups are 1-indexed
    for (int i = 1; i < noSecurities + 1; i++)
    {
        FIX44::SecurityList::NoRelatedSym security;
        message.getGroup(i, security);

        if (!m_writer.prepareMessage(m_securityDefinition))
        {
            std::cerr << "Error writing security definition" << std::endl;
            continue;
        }

        // Give the security an internal identifier
        const std::string& symbol = security.getField(FIX::FIELD::Symbol);
        securitiesInfo[securityIdCounter].symbol = symbol;
        securitiesInfo[securityIdCounter].status = com::liversedge::messages::SecurityStatusEnum::Value::NULL_VALUE;
        uint32_t id = securityIdCounter;
        m_securityDefinition.id(securityIdCounter);

        // Add to hash map for O(1) lookups
        m_symbolToSecurityId[symbol] = securityIdCounter;

        securityIdCounter++;

        // Add all security information
        m_securityDefinition.currency(SBEUtils::currencyFromString(security.getField(FIX::FIELD::Currency)));
        m_securityDefinition.commCurrency(SBEUtils::currencyFromString(security.getField(FIX::FIELD::CommCurrency)));
        m_securityDefinition.settlCurrency(SBEUtils::currencyFromString(security.getField(FIX::FIELD::SettlCurrency)));
        m_securityDefinition.settlType(SBEUtils::settlTypeFromString(security.getField(FIX::FIELD::SettlType)));
        SBEUtils::setDate(m_securityDefinition.maturityDate(), security.getField(FIX::FIELD::MaturityDate));
        SBEUtils::setPrice(m_securityDefinition.minPriceIncrement(), security.getField(FIX::FIELD::MinPriceIncrement));
        m_securityDefinition.instrumentPricePrecision(std::stoi(security.getField(FIXCustomTags::InstrumentPricePrecision)));
        SBEUtils::setQty(m_securityDefinition.minSizeIncrement(), security.getField(FIXCustomTags::MinSizeIncrement));
        SBEUtils::setPrice(m_securityDefinition.contractMultiplier(), security.getField(FIX::FIELD::ContractMultiplier));
        m_securityDefinition.securityType(SBEUtils::securityTypeFromString(security.getField(FIX::FIELD::SecurityType)));
        m_securityDefinition.timestamp(timestamp);

        // Variable length fields
        m_securityDefinition.symbol().wrap()
        SBEUtils::setVarString(m_securityDefinition, m_securityDefinition.symbol(), security.getField(FIX::FIELD::Symbol));

        // Write out security definition
        if (!m_writer.writeMessage(m_securityDefinition))
        {
            std::cerr << "Error writing security definition" << std::endl;
            securityIdCounter--;
            continue;
        }

        // Send the security status update
        if (!UpdateSecurityStatus(id, timestamp, com::liversedge::messages::SecurityStatusEnum::Value::PENDING_SNAPSHOT))
        {
            std::cerr << "Error updating security definition to PENDING_SNAPSHOT" << std::endl;
        }
    }
    m_writer.flush();
}

void MessageProcessor::onMessage(const FIX44::Logout& message, const FIX::SessionID& sessionID)
{
    std::cout << "Processing FIX44::Logout message" << std::endl;
    const uint64_t timestamp = GetSendingTime(static_cast<FIX44::Message>(message));
    InvalidateState(timestamp);
}

void MessageProcessor::onMessage(const FIX44::Logon& message, const FIX::SessionID& sessionID)
{
    std::cout << "Processing FIX44::Logon message" << std::endl;
    const uint64_t timestamp = GetSendingTime(static_cast<FIX44::Message>(message));

    if (securityIdCounter != 0)
    {
        // Indicates that the marketdata was not safely shutdown
        std::cerr << "Received Logon before Logout, invalidating the state" << std::endl;
        InvalidateState(timestamp);
    }

    // Send out ConnectionStatus - Offline
    UpdateConnectionStatus(com::liversedge::messages::ConnectionStatusEnum::ONLINE, timestamp);
}

void MessageProcessor::onMessage(const FIX44::MarketDataRequest& message, const FIX::SessionID& sessionID)
{
}

void MessageProcessor::onMessage(const FIX44::MarketDataRequestReject& message, const FIX::SessionID& sessionID)
{
}

bool MessageProcessor::InvalidateState(std::uint64_t timestamp)
{
    // Send out SecurityStatus - Offline for all securities
    for (int i = 0; i < securityIdCounter; i++)
    {
        UpdateSecurityStatus(i, timestamp, com::liversedge::messages::SecurityStatusEnum::Value::OFFLINE);
    }

    // Send out ConnectionStatus - Offline
    UpdateConnectionStatus(com::liversedge::messages::ConnectionStatusEnum::OFFLINE, timestamp);

    // Reset symbol state
    securityIdCounter = 0;
    m_symbolToSecurityId.clear();
    return true;
}

bool MessageProcessor::UpdateConnectionStatus(com::liversedge::messages::ConnectionStatusEnum::Value value, const std::uint64_t timestamp)
{
    if (!m_writer.prepareMessage(m_connectionStatus))
    {
        std::cerr << "Error preparing connection status update" << std::endl;
        return false;
    }

    m_connectionStatus.status(value);
    m_connectionStatus.timestamp(timestamp);

    if (!m_writer.writeMessage(m_connectionStatus))
    {
        std::cerr << "Error writing connection status update" << std::endl;
        return false;
    }

    return true;
}

bool MessageProcessor::UpdateSecurityStatus(int securityId, const std::uint64_t timestamp, com::liversedge::messages::SecurityStatusEnum::Value newStatus)
{
    if (securityId >= securityIdCounter)
    {
        std::cerr << "Error security " << securityId << "not found" << std::endl;
        return false;
    }
    if (securitiesInfo[securityId].status == newStatus)
    {
        // No change don't send
        return false;
    }
    if (!m_writer.prepareMessage(m_securityStatus))
    {
        std::cerr << "Error preparing security status update" << std::endl;
        return false;
    }

    m_securityStatus.securityId(securityId);
    m_securityStatus.timestamp(timestamp);
    m_securityStatus.status(newStatus);
    securitiesInfo[securityId].status = newStatus;

    if (!m_writer.writeMessage(m_securityStatus))
    {
        std::cerr << "Error writing security status update" << std::endl;
        return false;
    }

    return true;
}

uint64_t MessageProcessor::GetSendingTime(FIX44::Message message)
{
    FIX::SendingTime sendingTimeField;
    message.getHeader().get(sendingTimeField);
    FIX::UtcTimeStamp sendingTime = sendingTimeField.getValue();
    return FixUtils::convertFIXTimeToNanos(sendingTime);
}

template<typename T>
bool MessageProcessor::ProcessMDEntry(const T& entry, int securityId, uint64_t timestamp)
{
    // Prepare MDUpdate message
    if (!m_writer.prepareMessage(m_mdUpdate))
    {
        std::cerr << "Error preparing MDUpdate message" << std::endl;
        return false;
    }

    m_mdUpdate.securityId(securityId);
    m_mdUpdate.timestamp(timestamp);

    // Get entry type and classify
    FIX::MDEntryType mdEntryType;
    entry.get(mdEntryType);

    // Extract MDUpdateAction for book updates
    com::liversedge::messages::MDUpdateAction::Value updateAction = com::liversedge::messages::MDUpdateAction::Value::NULL_VALUE;
    if (entry.isSetField(FIX::FIELD::MDUpdateAction))
    {
        const std::string& actionStr = entry.getField(FIX::FIELD::MDUpdateAction);
        if (actionStr == "0") {
            updateAction = com::liversedge::messages::MDUpdateAction::Value::NEW;
        } else if (actionStr == "1") {
            updateAction = com::liversedge::messages::MDUpdateAction::Value::CHANGE;
        } else if (actionStr == "2") {
            updateAction = com::liversedge::messages::MDUpdateAction::Value::DELETE;
        }
    }

    if (mdEntryType.getValue() == FIX::MDEntryType_BID)
    {
        // Bid book update
        m_mdUpdate.updateType(com::liversedge::messages::MDUpdateType::BOOK_UPDATE);
        m_mdUpdate.side(com::liversedge::messages::MDSide::BID);
        m_mdUpdate.action(updateAction);
        SBEUtils::setPrice(m_mdUpdate.price(), entry.getField(FIX::FIELD::MDEntryPx));
        SBEUtils::setQty(m_mdUpdate.qty(), entry.getField(FIX::FIELD::MDEntrySize));
    }
    else if (mdEntryType.getValue() == FIX::MDEntryType_OFFER)
    {
        // Ask book update
        m_mdUpdate.updateType(com::liversedge::messages::MDUpdateType::BOOK_UPDATE);
        m_mdUpdate.side(com::liversedge::messages::MDSide::ASK);
        m_mdUpdate.action(updateAction);
        SBEUtils::setPrice(m_mdUpdate.price(), entry.getField(FIX::FIELD::MDEntryPx));
        SBEUtils::setQty(m_mdUpdate.qty(), entry.getField(FIX::FIELD::MDEntrySize));
    }
    else if (mdEntryType.getValue() == FIX::MDEntryType_TRADE)
    {
        // Trade execution
        m_mdUpdate.updateType(com::liversedge::messages::MDUpdateType::TRADE);
        SBEUtils::setPrice(m_mdUpdate.price(), entry.getField(FIX::FIELD::MDEntryPx));
        SBEUtils::setQty(m_mdUpdate.qty(), entry.getField(FIX::FIELD::MDEntrySize));

        // Set trade side (what the taker was doing)
        FIX::Side side;
        entry.getField(side);
        m_mdUpdate.side(side.getValue() == FIX::Side_BUY ? com::liversedge::messages::MDSide::ASK : com::liversedge::messages::MDSide::BID);

        // Set trade ID if available (optional field)
        if (entry.isSetField(FIX::FIELD::MDEntryID))
        {
            m_mdUpdate.tradeId(std::stoull(entry.getField(FIX::FIELD::MDEntryID)));
        }
    }
    else
    {
        // Unknown entry type, skip
        return false;
    }

    // Write the MDUpdate message
    if (!m_writer.writeMessage(m_mdUpdate))
    {
        std::cerr << "Error writing MDUpdate message" << std::endl;
        return false;
    }

    return true;
}

// Explicit template instantiations
template bool MessageProcessor::ProcessMDEntry<FIX44::MarketDataIncrementalRefresh::NoMDEntries>(const FIX44::MarketDataIncrementalRefresh::NoMDEntries& entry, int securityId, uint64_t timestamp);
template bool MessageProcessor::ProcessMDEntry<FIX44::MarketDataSnapshotFullRefresh::NoMDEntries>(const FIX44::MarketDataSnapshotFullRefresh::NoMDEntries& entry, int securityId, uint64_t timestamp);

