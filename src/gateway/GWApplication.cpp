#include "../../include/gateway/GWApplication.h"

GWApplication::GWApplication(SimpleConfig& config, RefDataHolder& refDataHolder)
    : m_config(config), m_refDataHolder(refDataHolder)
{
    m_sbeWriter = std::make_unique<SBEBinaryWriter>();
    std::string outboundPath = config.getString("gw_outbound_file_path");
    m_sbeWriter->openNewFile(outboundPath + kPathSeparator + "messages.sbe", true); // true = append mode
}

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


void GWApplication::onMessage(const FIX44::OrderCancelReject& message, const FIX::SessionID& sessionID) {
    FixUtils::logFixMessage("Received OrderCancelReject: ", message);

    // Convert FIX OrderCancelReject to SBE and write to outbound file
    com::liversedge::messages::OrderCancelReject sbeReject;
    if (m_sbeWriter->prepareMessage(sbeReject)) {
        // Extract fields from FIX message
        std::string clientOrderId = message.getField(FIX::FIELD::ClOrdID);
        std::string origClientOrderId = message.getField(FIX::FIELD::OrigClOrdID);
        std::string symbol = message.getField(FIX::FIELD::Symbol);

        // Map symbol to security ID
        std::int32_t securityId = m_refDataHolder.getSecurityIdBySymbol(symbol);

        // Set SBE fields
        sbeReject.timestamp(extractSendingTimeFromFix(message));
        sbeReject.securityId(securityId);
        SBEUtils::setVarString(sbeReject, sbeReject.clientOrderId(), clientOrderId);
        SBEUtils::setVarString(sbeReject, sbeReject.origClientOrderId(), origClientOrderId);
        // TODO: add reject reason and reject reason string

        m_sbeWriter->writeMessage(sbeReject);
        m_sbeWriter->flush();
    }
}

void GWApplication::onMessage(const FIX44::ExecutionReport& message, const FIX::SessionID& sessionID) {
    FixUtils::logFixMessage("Received ExecutionReport: ", message);

    // Convert FIX ExecutionReport to SBE and write to outbound file
    com::liversedge::messages::ExecutionReport sbeExecReport;
    if (m_sbeWriter->prepareMessage(sbeExecReport)) {
        // Extract fields from FIX message
        std::string clientOrderId = message.getField(FIX::FIELD::ClOrdID);
        std::string origClientOrderId = message.getField(FIX::FIELD::OrigClOrdID);
        std::string symbol = message.getField(FIX::FIELD::Symbol);

        // Map symbol to security ID
        std::int32_t securityId = m_refDataHolder.getSecurityIdBySymbol(symbol);

        // Set SBE fields
        sbeExecReport.timestamp(extractSendingTimeFromFix(message));
        FIX::TransactTime transactTime;
        message.getField(transactTime);
        sbeExecReport.timestamp(transactTime.getValue().getTimeT() * 1000000000ULL);
        sbeExecReport.securityId(securityId);
        FIX::OrdStatus ordStatus{};
        message.get(ordStatus);
        sbeExecReport.ordStatus(SBEUtils::ordStatusFromFix(ordStatus));
        FIX::Side side{};
        message.get(side);
        sbeExecReport.side(SBEUtils::sideFromFix(side));
        FIX::OrdRejReason ordRejReason{};
        message.get(ordRejReason);
        sbeExecReport.ordRejReason(SBEUtils::ordRejReasonFromFix(ordRejReason));
        sbeExecReport.securityId(securityId);
        SBEUtils::setQty(sbeExecReport.leavesQty(), message.getField(FIX::FIELD::LeavesQty));
        SBEUtils::setQty(sbeExecReport.cumQty(), message.getField(FIX::FIELD::CumQty));
        SBEUtils::setQty(sbeExecReport.orderQty(), message.getField(FIX::FIELD::OrderQty));
        if (message.isSetField(FIX::FIELD::Price))
        {
            SBEUtils::setPrice(sbeExecReport.price(), message.getField(FIX::FIELD::Price));
        }
        if (message.isSetField(FIX::FIELD::LastPx))
        {
            SBEUtils::setPrice(sbeExecReport.lastPx(), message.getField(FIX::FIELD::Price));
        }
        if (message.isSetField(FIX::FIELD::LastQty))
        {
            SBEUtils::setQty(sbeExecReport.lastQty(), message.getField(FIX::FIELD::LastQty));
        }
        SBEUtils::setVarString(sbeExecReport, sbeExecReport.origClientOrderId(), origClientOrderId);
        SBEUtils::setVarString(sbeExecReport, sbeExecReport.clientOrderId(), clientOrderId);
        if (message.isSetField(FIX::FIELD::Text))
        {
            SBEUtils::setVarString(sbeExecReport, sbeExecReport.text(), message.getField(FIX::FIELD::Text));
        } else
        {
            SBEUtils::setVarString(sbeExecReport, sbeExecReport.text(), "");
        }

        m_sbeWriter->writeMessage(sbeExecReport);
        m_sbeWriter->flush();
    }
}

bool GWApplication::sendMessage(FIX::Message& message)
{
    if (!m_loggedOn) {
        std::cout << "Cannot send message: not logged on" << std::endl;
        return false;
    }

    try {
        FIX::Session::sendToTarget(message, m_sessionID);
        return true;
    } catch (const std::exception& e) {
        std::cout << "Failed to send message: " << e.what() << std::endl;
        return false;
    }
}

std::uint64_t GWApplication::extractSendingTimeFromFix(const FIX::Message& message) const
{
    try {
        FIX::SendingTime sendingTime;
        message.getHeader().getField(sendingTime);
        // Convert from time_t to nanoseconds (time_t is seconds since epoch)
        return sendingTime.getValue().getTimeT() * 1000000000ULL;
    } catch (const std::exception& e) {
        // Fallback to current time if SendingTime is not available
        std::cout << "Warning: SendingTime not found in FIX message, using current time" << std::endl;
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    }
}

