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

    int securityId = -1;
    const std::string& symbol = message.getField(FIX::FIELD::Symbol);
    for (int i = 0; i < securityIdCounter; i++)
    {
        if (securitiesInfo[i].symbol == symbol)
        {
            securityId = i;
            break;
        }
    }
    if (securityId == -1)
    {
        std::cerr << "No matching security found for " << symbol << std::endl;
        return;
    }

    // TODO: Publish initial order book as snapshot - app should process a snapshot before it starts up

    // Send out SecurityStatus - Online afterwards
    UpdateSecurityStatus(securityId, timestamp, com::liversedge::messages::SecurityStatusEnum::Value::ONLINE);
}

void MessageProcessor::onMessage(const FIX44::MarketDataIncrementalRefresh& message, const FIX::SessionID& sessionID)
{
    // Parse incremental refreshes into trades and book updates
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
        securitiesInfo[securityIdCounter].symbol = security.getField(FIX::FIELD::Symbol);
        securitiesInfo[securityIdCounter].status = com::liversedge::messages::SecurityStatusEnum::Value::NULL_VALUE;
        uint32_t id = securityIdCounter;
        m_securityDefinition.id(securityIdCounter);
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
        SBEUtils::setVarString(m_securityDefinition.symbol(), security.getField(FIX::FIELD::Symbol));

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

