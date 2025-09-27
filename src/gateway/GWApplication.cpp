#include "../../include/gateway/GWApplication.h"

void GWApplication::onCreate(const FIX::SessionID& sessionID)
{
    std::cout << "Session created: " << sessionID << std::endl;
    m_sessionID = sessionID;
}

void GWApplication::onLogon(const FIX::SessionID& sessionID)
{
    std::cout << "Logged on to Deribit: " << sessionID << std::endl;
    m_loggedOn = true;
}

void GWApplication::onLogout(const FIX::SessionID& sessionID)
{
    std::cout << "Logged out from Deribit: " << sessionID << std::endl;
    m_loggedOn = false;
}

void GWApplication::toAdmin(FIX::Message& message, const FIX::SessionID& sessionID)
{
    // Add authentication for logon messages
    FIX::MsgType msgType;
    message.getHeader().getField(msgType);

    if (msgType.getValue() == FIX::MsgType_Logon)
    {
        FixUtils::addDeribitAuth(message, m_config);
    }
}

void GWApplication::toApp(FIX::Message& message, const FIX::SessionID& sessionID) noexcept
{
    FixUtils::logFixMessage("Sending: ", message);
}

void GWApplication::fromAdmin(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept
{
    FixUtils::logFixMessage("Received admin: ", message);
}

void GWApplication::fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept
{    
    // This automatically calls down to the corresponding onMessage implementation
    crack(message, sessionID);
}


void GWApplication::onMessage(const FIX44::MarketDataRequest& message, const FIX::SessionID& sessionID) {
    // logFixMessage("Received market data request: ", message);
}

