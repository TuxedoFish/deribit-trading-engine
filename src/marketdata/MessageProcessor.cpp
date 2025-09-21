#include "../../include/marketdata/MessageProcessor.h"

MessageProcessor::MessageProcessor(SBEBinaryWriter& writer) : m_writer(writer) {
}

void MessageProcessor::onMessage(const FIX44::MarketDataSnapshotFullRefresh& message, const FIX::SessionID& sessionID) {
    // Publish initial order book as snapshot - app should process a snapshot before it starts up
    // Send out SecurityStatus - Online afterwards
}

void MessageProcessor::onMessage(const FIX44::MarketDataIncrementalRefresh& message, const FIX::SessionID& sessionID) {
    // Parse incremental refreshes into trades and book updates
}

void MessageProcessor::onMessage(const FIX44::SecurityList& message, const FIX::SessionID& sessionID) {
    // Build up state for each symbol
    // Publish security definitions
    FIX::NoRelatedSym noSecuritiesField;
    message.get(noSecuritiesField);
    int noSecurities = noSecuritiesField.getValue();

    // FIX repeating groups are 1-indexed
    for (int i = 1; i < noSecurities + 1; i++) {
        FIX44::SecurityList::NoRelatedSym security;
        message.getGroup(i, security);
        FIX::Symbol symbol;
        security.get(symbol);

        // Write out security definition
        m_writer.writeSecurityDefinition(symbol.getString());
    }
    m_writer.flush();
}

void MessageProcessor::onMessage(const FIX44::Logout& message, const FIX::SessionID& sessionID) {
    // Send out SecurityStatus - Offline for all securities
}

void MessageProcessor::onMessage(const FIX44::MarketDataRequest& message, const FIX::SessionID& sessionID) {
}

void MessageProcessor::onMessage(const FIX44::MarketDataRequestReject& message, const FIX::SessionID& sessionID) {
}

