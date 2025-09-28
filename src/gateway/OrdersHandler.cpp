#include "../../include/gateway/OrdersHandler.h"
#include "../../include/gateway/GWApplication.h"
#include "../../include/sbe/SBEUtils.h"
#include <iostream>

OrdersHandler::OrdersHandler(std::unique_ptr<RefDataHolder> refDataHolder, GWApplication& gwApplication)
    : m_refDataHolder(std::move(refDataHolder)), m_gwApplication(gwApplication)
{
}

void OrdersHandler::onSecurityDefinition(com::liversedge::messages::SecurityDefinition& decoder, std::uint64_t timestamp)
{
    m_refDataHolder->onSecurityDefinition(decoder, timestamp);
}

void OrdersHandler::onNewOrder(com::liversedge::messages::NewOrder& decoder, std::uint64_t timestamp)
{
    if (m_isReplay)
    {
        return;
    }

    try {
        std::int32_t securityId = decoder.securityId();
        auto side = decoder.side();
        auto orderType = decoder.orderType();
        auto timeInForce = decoder.timeInForce();
        auto price = SBEUtils::convertPrice(decoder.price());
        auto quantity = SBEUtils::convertQty(decoder.quantity());
        std::string clientOrderId = SBEUtils::extractVarString(decoder.clientOrderId(), decoder.sbeBlockLength());

        const SecurityInfo* secInfo = m_refDataHolder->getSecurityInfo(securityId);
        if (!secInfo) {
            std::cout << "OrdersHandler: Security not found for ID: " << securityId << std::endl;
            return;
        }

        FIX44::NewOrderSingle newOrderSingle;

        newOrderSingle.setField(FIX::ClOrdID(clientOrderId));
        newOrderSingle.setField(FIX::Symbol(secInfo->getSymbol()));
        newOrderSingle.setField(SBEUtils::convertSide(side));
        newOrderSingle.setField(FIX::OrderQty(quantity.convert_to<double>()));
        newOrderSingle.setField(SBEUtils::convertOrderType(orderType));

        if (orderType != com::liversedge::messages::OrderType::MARKET) {
            newOrderSingle.setField(FIX::Price(price.convert_to<double>()));
        }

        newOrderSingle.setField(SBEUtils::convertTimeInForce(timeInForce));

        if (m_gwApplication.sendMessage(newOrderSingle)) {
            std::cout << "OrdersHandler: Sent NewOrderSingle for " << clientOrderId
                      << " (" << secInfo->getSymbol() << ")" << std::endl;
        } else {
            std::cout << "OrdersHandler: Failed to send NewOrderSingle for " << clientOrderId << std::endl;
        }

    } catch (const std::exception& e) {
        std::cout << "OrdersHandler: Error processing NewOrder: " << e.what() << std::endl;
    }
}

void OrdersHandler::onCancelOrder(com::liversedge::messages::CancelOrder& decoder, std::uint64_t timestamp)
{
    if (m_isReplay)
    {
        return;
    }

    try {
        std::int32_t securityId = decoder.securityId();
        std::string clientOrderId = SBEUtils::extractVarString(decoder.clientOrderId(), decoder.sbeBlockLength());
        std::string origClientOrderId = SBEUtils::extractVarString(decoder.origClientOrderId(), decoder.sbeBlockLength(), clientOrderId.length());

        const SecurityInfo* secInfo = m_refDataHolder->getSecurityInfo(securityId);
        if (!secInfo) {
            std::cout << "OrdersHandler: Security not found for ID: " << securityId << std::endl;
            return;
        }

        FIX44::OrderCancelRequest cancelRequest;

        cancelRequest.setField(FIX::ClOrdID(clientOrderId));
        cancelRequest.setField(FIX::OrigClOrdID(origClientOrderId));
        cancelRequest.setField(FIX::Symbol(secInfo->getSymbol()));

        if (m_gwApplication.sendMessage(cancelRequest)) {
            std::cout << "OrdersHandler: Sent OrderCancelRequest for " << origClientOrderId
                      << " (" << secInfo->getSymbol() << ")" << std::endl;
        } else {
            std::cout << "OrdersHandler: Failed to send OrderCancelRequest for " << origClientOrderId << std::endl;
        }

    } catch (const std::exception& e) {
        std::cout << "OrdersHandler: Error processing CancelOrder: " << e.what() << std::endl;
    }
}

