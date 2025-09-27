#include "../../include/gateway/OrdersHandler.h"
#include <iostream>

OrdersHandler::OrdersHandler(std::unique_ptr<RefDataHolder> refDataHolder)
    : m_refDataHolder(std::move(refDataHolder))
{
}

void OrdersHandler::onSecurityDefinition(const com::liversedge::messages::SecurityDefinition& decoder, std::uint64_t timestamp)
{
    // Delegate to RefDataHolder
    m_refDataHolder->onSecurityDefinition(decoder, timestamp);
}

void OrdersHandler::onNewOrder(const com::liversedge::messages::NewOrder& decoder, std::uint64_t timestamp)
{
    if (m_isReplay)
    {
        return;
    }
    std::cout << "OrdersHandler: Received NewOrder" << std::endl;
    // TODO: Implement order handling logic
}

void OrdersHandler::onCancelOrder(const com::liversedge::messages::CancelOrder& decoder, std::uint64_t timestamp)
{
    if (m_isReplay)
    {
        return;
    }
    std::cout << "OrdersHandler: Received CancelOrder" << std::endl;
    // TODO: Implement cancel order handling logic
}