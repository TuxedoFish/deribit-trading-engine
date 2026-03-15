#include "../../include/gateway/HyperliquidOrdersHandler.h"
#include <spdlog/spdlog.h>

HyperliquidOrdersHandler::HyperliquidOrdersHandler(RefDataHolder& refDataHolder, HyperliquidGWApplication& gwApplication, SBEBinaryWriter& sbeWriter)
    : m_refDataHolder(refDataHolder), m_gwApplication(gwApplication), m_sbeWriter(sbeWriter)
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

    try {
        std::int32_t securityId = decoder.securityId();
        auto side = decoder.side();
        auto timeInForce = decoder.timeInForce();
        auto price = SBEUtils::convertPrice(decoder.price());
        auto quantity = SBEUtils::convertQty(decoder.quantity());
        std::string clientOrderId = SBEUtils::extractVarString(decoder.clientOrderId(), decoder.sbeBlockLength());

        const SecurityInfo* secInfo = m_refDataHolder.getSecurityInfo(securityId);
        if (!secInfo) {
            spdlog::error("OrdersHandler: Security not found for ID: {}", securityId);
            sendNewOrderReject(decoder);
            return;
        }

        if (!m_gwApplication.isConnected()) {
            spdlog::error("OrdersHandler: Not connected, rejecting order {}", clientOrderId);
            sendNewOrderReject(decoder);
            return;
        }

        hyperliquid::OrderRequest order;
        order.asset = secInfo->getSymbol();
        order.isBuy = (side == com::liversedge::messages::Side::BUY);
        order.price = price.convert_to<double>();
        order.size = quantity.convert_to<double>();
        order.reduceOnly = false;
        order.limit = hyperliquid::LimitOrderType{mapTimeInForce(timeInForce)};
        order.cloid = clientOrderId;

        m_gwApplication.getWebsocket().placeOrder({order}, hyperliquid::Grouping::Na);
        spdlog::info("OrdersHandler: Sent placeOrder for {} ({})", clientOrderId, secInfo->getSymbol());

    } catch (const std::exception& e) {
        spdlog::error("OrdersHandler: Error processing NewOrder: {}", e.what());
        sendNewOrderReject(decoder);
    }
}

void HyperliquidOrdersHandler::onAmendOrder(com::liversedge::messages::AmendOrder& decoder, std::uint64_t timestamp)
{
    if (m_isReplay)
    {
        return;
    }

    try {
        std::int32_t securityId = decoder.securityId();
        auto side = decoder.side();
        auto timeInForce = decoder.timeInForce();
        auto price = SBEUtils::convertPrice(decoder.price());
        auto quantity = SBEUtils::convertQty(decoder.quantity());
        std::string clientOrderId = SBEUtils::extractVarString(decoder.clientOrderId(), decoder.sbeBlockLength());

        const SecurityInfo* secInfo = m_refDataHolder.getSecurityInfo(securityId);
        if (!secInfo) {
            spdlog::error("OrdersHandler: Security not found for ID: {}", securityId);
            return;
        }

        if (!m_gwApplication.isConnected()) {
            spdlog::error("OrdersHandler: Not connected, cannot amend {}", clientOrderId);
            return;
        }

        hyperliquid::OrderRequest order;
        order.asset = secInfo->getSymbol();
        order.isBuy = (side == com::liversedge::messages::Side::BUY);
        order.price = price.convert_to<double>();
        order.size = quantity.convert_to<double>();
        order.reduceOnly = false;
        order.limit = hyperliquid::LimitOrderType{mapTimeInForce(timeInForce)};
        order.cloid = clientOrderId;

        hyperliquid::ModifyRequest modify;
        modify.cloid = clientOrderId;
        modify.order = order;

        m_gwApplication.getWebsocket().modifyOrder(modify);
        spdlog::info("OrdersHandler: Sent modifyOrder for {} ({})", clientOrderId, secInfo->getSymbol());

    } catch (const std::exception& e) {
        spdlog::error("OrdersHandler: Error processing AmendOrder: {}", e.what());
    }
}

void HyperliquidOrdersHandler::onCancelOrder(com::liversedge::messages::CancelOrder& decoder, std::uint64_t timestamp)
{
    if (m_isReplay)
    {
        return;
    }

    try {
        std::int32_t securityId = decoder.securityId();
        std::string clientOrderId = SBEUtils::extractVarString(decoder.clientOrderId(), decoder.sbeBlockLength());
        std::string origClientOrderId = SBEUtils::extractVarString(decoder.origClientOrderId(), decoder.sbeBlockLength(), clientOrderId.length());

        const SecurityInfo* secInfo = m_refDataHolder.getSecurityInfo(securityId);
        if (!secInfo) {
            spdlog::error("OrdersHandler: Security not found for ID: {}", securityId);
            sendCancelReject(decoder);
            return;
        }

        if (!m_gwApplication.isConnected()) {
            spdlog::error("OrdersHandler: Not connected, cannot cancel {}", origClientOrderId);
            sendCancelReject(decoder);
            return;
        }

        hyperliquid::CancelByCloidRequest cancel;
        cancel.asset = secInfo->getSymbol();
        cancel.cloid = origClientOrderId;

        m_gwApplication.getWebsocket().cancelOrderByCloid({cancel});
        spdlog::info("OrdersHandler: Sent cancelOrderByCloid for {} ({})", origClientOrderId, secInfo->getSymbol());

    } catch (const std::exception& e) {
        spdlog::error("OrdersHandler: Error processing CancelOrder: {}", e.what());
        sendCancelReject(decoder);
    }
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

hyperliquid::Tif HyperliquidOrdersHandler::mapTimeInForce(com::liversedge::messages::TimeInForce::Value tif)
{
    switch (tif)
    {
    case com::liversedge::messages::TimeInForce::GTC:
        return hyperliquid::Tif::Gtc;
    case com::liversedge::messages::TimeInForce::IOC:
        return hyperliquid::Tif::Ioc;
    case com::liversedge::messages::TimeInForce::FOK:
    default:
        return hyperliquid::Tif::Alo;
    }
}
