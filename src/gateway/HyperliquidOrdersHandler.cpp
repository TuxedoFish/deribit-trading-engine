#include "../../include/gateway/HyperliquidOrdersHandler.h"

HyperliquidOrdersHandler::HyperliquidOrdersHandler(RefDataHolder& refDataHolder, SBEBinaryWriter& sbeWriter)
    : m_refDataHolder(refDataHolder), m_sbeWriter(sbeWriter)
{
}

void HyperliquidOrdersHandler::onSecurityDefinition(com::liversedge::messages::SecurityDefinition& decoder, std::uint64_t timestamp)
{
    m_refDataHolder.onSecurityDefinition(decoder, timestamp);
}

void HyperliquidOrdersHandler::onNewOrder(com::liversedge::messages::NewOrder& decoder, std::uint64_t timestamp)
{
    if (m_isReplay)
    {
        return;
    }
    std::cout << "New order received: " << decoder << std::endl;
}

void HyperliquidOrdersHandler::onAmendOrder(com::liversedge::messages::AmendOrder& decoder, std::uint64_t timestamp)
{
    if (m_isReplay)
    {
        return;
    }
    std::cout << "Amend order received: " << decoder << std::endl;
}

void HyperliquidOrdersHandler::onCancelOrder(com::liversedge::messages::CancelOrder& decoder, std::uint64_t timestamp)
{
    if (m_isReplay)
    {
        return;
    }
    std::cout << "Cancel order received: " << decoder << std::endl;
}

void HyperliquidOrdersHandler::sendCancelReject(com::liversedge::messages::CancelOrder& cancelOrder)
{
    com::liversedge::messages::OrderCancelReject sbeReject;
    if (m_sbeWriter.prepareMessage(sbeReject)) {
        DeribitMessageConverter::createInternalOrderCancelReject(cancelOrder, sbeReject);
        m_sbeWriter.writeMessage(sbeReject);
    }
}

void HyperliquidOrdersHandler::sendNewOrderReject(com::liversedge::messages::NewOrder& newOrder)
{
    com::liversedge::messages::ExecutionReport sbeExecReport;
    if (m_sbeWriter.prepareMessage(sbeExecReport)) {
        DeribitMessageConverter::createNewOrderReject(newOrder, sbeExecReport);
        m_sbeWriter.writeMessage(sbeExecReport);
    }
}

