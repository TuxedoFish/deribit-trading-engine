#include "../../include/marketdata/MessageProcessor.h"

MessageProcessor::MessageProcessor(SBEBinaryWriter& writer)
    : m_writer(writer), securityIdCounter(1)
{
}

void MessageProcessor::onMessage(const FIX44::MarketDataSnapshotFullRefresh& message, const FIX::SessionID& sessionID)
{
    // Publish initial order book as snapshot - app should process a snapshot before it starts up
    // Send out SecurityStatus - Online afterwards
}

void MessageProcessor::onMessage(const FIX44::MarketDataIncrementalRefresh& message, const FIX::SessionID& sessionID)
{
    // Parse incremental refreshes into trades and book updates
}

void MessageProcessor::onMessage(const FIX44::SecurityList& message, const FIX::SessionID& sessionID)
{
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

        // Variable length fields
        SBEUtils::setVarString(m_securityDefinition.symbol(), security.getField(FIX::FIELD::Symbol));

        // Write out security definition
        if (!m_writer.writeMessage(m_securityDefinition))
        {
            std::cerr << "Error writing security definition" << std::endl;
        }
    }
    m_writer.flush();
}

void MessageProcessor::onMessage(const FIX44::Logout& message, const FIX::SessionID& sessionID)
{
    // Send out SecurityStatus - Offline for all securities
}

void MessageProcessor::onMessage(const FIX44::MarketDataRequest& message, const FIX::SessionID& sessionID)
{
}

void MessageProcessor::onMessage(const FIX44::MarketDataRequestReject& message, const FIX::SessionID& sessionID)
{
}

bool MessageProcessor::isMultileg(const std::string& symbol)
{
    if (symbol.find("-FS-") != std::string::npos)
    {
        // Future spread
        return true;
    }
    return false;
}
