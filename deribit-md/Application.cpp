#include "Application.h"

void Application::onCreate(const FIX::SessionID& sessionID)
{
    std::cout << "Session created: " << sessionID << std::endl;
    m_sessionID = sessionID;
}

void Application::onLogon(const FIX::SessionID& sessionID)
{
    std::cout << "Logged on to Deribit: " << sessionID << std::endl;
    m_loggedOn = true;
    getSymbols();
}

void Application::onLogout(const FIX::SessionID& sessionID)
{
    std::cout << "Logged out from Deribit: " << sessionID << std::endl;
    m_loggedOn = false;
}

void Application::toAdmin(FIX::Message& message, const FIX::SessionID& sessionID)
{
    // Add authentication for logon messages
    FIX::MsgType msgType;
    message.getHeader().getField(msgType);

    if (msgType.getValue() == FIX::MsgType_Logon)
    {
        // TODO: Fetch from a settings file
        // string user = settings->get().getString("Username");
        // string secret = settings->get().getString("Password");
        std::string user = "CsNBskWa";
        std::string secret = "ZE2W9NCqp7QtbWI7avjML4T0DANpDCR2t_MgLnHiNTY";

        // Creates the raw data
        std::chrono::milliseconds ms = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        std::string timestamp_in_ms = std::to_string(ms.count());
        unsigned char nonce[32] = {};
        RAND_bytes(nonce, sizeof(nonce));
        std::string nonce64 = AuthHandler::base64_encode(nonce, sizeof(nonce));
        std::string raw_data = timestamp_in_ms + "." + nonce64;
        std::string base_signature_string = raw_data + secret;
        std::cout << "Logging on with timetsamp=" << timestamp_in_ms << " nonce64=" << nonce64 << " raw_data=" << raw_data << "base_signature_string=" << base_signature_string << std::endl;

        // Creates the password field
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, base_signature_string.c_str(), base_signature_string.size());
        SHA256_Final(hash, &sha256);
        static std::string password_sha_base64 = AuthHandler::base64_encode(hash, sizeof(hash));

        message.setField(FIX::Username(user));
        message.setField(FIX::RawData(raw_data));
        message.setField(FIX::Password(password_sha_base64));
        std::cout << "Sending logon with credentials" << std::endl;
    }
}

void Application::toApp(FIX::Message& message, const FIX::SessionID& sessionID) noexcept
{
    logFixMessage("Sending: ", message);
}

void Application::fromAdmin(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept
{
    logFixMessage("Received admin: ", message);
}

void Application::fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept
{    
    // This automatically calls down to the corresponding onMessage implementation
    crack(message, sessionID);
}

void Application::getSymbols()
{
    // Create SecurityListRequest message
    FIX::Message secListRequest;
    secListRequest.getHeader().setField(FIX::MsgType("x")); // SecurityListRequest

    // Required fields
    secListRequest.setField(FIX::SecurityReqID("SYMBOLS_001"));
    secListRequest.setField(FIX::SecurityListRequestType(4)); // All Securities
    secListRequest.setField(FIX::Currency("BTC"));
    secListRequest.setField(FIX::SecurityType("FUT"));  // Futures

    try {
        FIX::Session::sendToTarget(secListRequest, m_sessionID);
    }
    catch (const std::exception& e) {
        std::cout << "Error sending SecurityListRequest: " << e.what() << std::endl;
    }

    secListRequest.setField(FIX::SecurityReqID("SYMBOLS_002"));
    secListRequest.setField(FIX::SecurityType("FUTCO"));  // Futures

    try {
        FIX::Session::sendToTarget(secListRequest, m_sessionID);
    }
    catch (const std::exception& e) {
        std::cout << "Error sending SecurityListRequest: " << e.what() << std::endl;
    }
}

void Application::subscribe(std::string symbols[], int nSymbols)
{
    // Create market data request with default constructor
    FIX44::MarketDataRequest mdRequest;

    // Set required fields individually
    mdRequest.set(FIX::MDReqID("1"));
    mdRequest.set(FIX::SubscriptionRequestType(FIX::SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES));
    mdRequest.set(FIX::MarketDepth(0));
    mdRequest.set(FIX::MDUpdateType(FIX::MDUpdateType_INCREMENTAL_REFRESH));
    
    // Set group counts BEFORE adding groups
    mdRequest.set(FIX::NoMDEntryTypes(3));  // Bid, Ask & Trades
    mdRequest.set(FIX::NoRelatedSym(1));    // One symbol

    // Add entry types
    FIX44::MarketDataRequest::NoMDEntryTypes bidEntryGroup;
    bidEntryGroup.set(FIX::MDEntryType(FIX::MDEntryType_BID));
    mdRequest.addGroup(bidEntryGroup);

    FIX44::MarketDataRequest::NoMDEntryTypes askEntryGroup;
    askEntryGroup.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
    mdRequest.addGroup(askEntryGroup);

    FIX44::MarketDataRequest::NoMDEntryTypes tradeEntryGroup;
    tradeEntryGroup.set(FIX::MDEntryType(FIX::MDEntryType_TRADE));
    mdRequest.addGroup(tradeEntryGroup);

    // Add symbol
    FIX44::MarketDataRequest::NoRelatedSym symbolGroup;
    for (int i = 0; i < nSymbols; i++) {
        symbolGroup.set(FIX::Symbol(symbols[i]));
        mdRequest.addGroup(symbolGroup);
    }

    // Send the request
    FIX::Session::sendToTarget(mdRequest, m_sessionID);
}

void Application::onMessage(const FIX44::MarketDataRequest& message, const FIX::SessionID& sessionID) {
    // logFixMessage("Received market data request: ", message);
}

void Application::onMessage(const FIX44::MarketDataRequestReject& message, const FIX::SessionID& sessionID) {
    // std::cout << "received market data request reject" << std::endl;
}

void Application::onMessage(const FIX44::MarketDataSnapshotFullRefresh& message, const FIX::SessionID& sessionID) {
    FIX::Symbol symbol;
    message.get(symbol);
    std::cout << "Received MarketDataSnapshotFullRefresh for: " << symbol.getValue() << std::endl;
}

void Application::onMessage(const FIX44::MarketDataIncrementalRefresh& message, const FIX::SessionID& sessionID) {
    // std::cout << "received market data incremental refresh" << std::endl;
}

void Application::onMessage(const FIX44::SecurityList& message, const FIX::SessionID& sessionID) {
    logFixMessage("Received SecurityList: ", message);

    FIX::NoRelatedSym noSecuritiesField;
    message.get(noSecuritiesField);
    int noSecurities = noSecuritiesField.getValue();
    std::string* symbols = new std::string[noSecurities];

    // FIX repeating groups are 1-indexed
    for (int i = 1; i < noSecurities + 1; i++) {
        FIX44::SecurityList::NoRelatedSym security;
        message.getGroup(i, security);
        FIX::Symbol symbol;
        security.get(symbol);
        symbols[i-1] = symbol.getString();
    }

    std::cout << "Symbols: ";
    for (int i = 0; i < noSecurities; i++) {
        std::cout << symbols[i] << ", ";
    }
    std::cout << std::endl;
    Application::subscribe(symbols, noSecurities);
}

void Application::logFixMessage(const std::string& prefix, const FIX::Message& message)
{
    std::string fixString = message.toString();
    std::replace(fixString.begin(), fixString.end(), '\001', '|');
    std::cout << prefix << ": " << fixString << std::endl;
}
