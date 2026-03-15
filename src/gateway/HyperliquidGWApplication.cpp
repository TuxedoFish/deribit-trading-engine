#include "../../include/gateway/HyperliquidGWApplication.h"
#include <chrono>
#include <spdlog/spdlog.h>

HyperliquidGWApplication::HyperliquidGWApplication(SimpleConfig& config, RefDataHolder& refDataHolder, SBEBinaryWriter& sbeWriter)
    : m_config(config), m_refDataHolder(refDataHolder), m_sbeWriter(sbeWriter), m_restParser(*this)
{
}

void HyperliquidGWApplication::start()
{
    m_apiConfig.env = getEnvironment(m_config.getString("environment"));
    m_apiConfig.wallet = hyperliquid::Wallet{
        m_config.getString("hl_account_address"),
        m_config.getString("hl_private_key")
    };

    m_websocket = std::make_unique<hyperliquid::WebsocketApi>(m_apiConfig, *this);
    m_websocket->start();
}

void HyperliquidGWApplication::stop()
{
    if (m_websocket)
    {
        m_websocket->stop();
    }
}

// WebsocketApiListener

void HyperliquidGWApplication::onMessage(const std::string& message)
{
    m_wsParser.crack(message, *this);
}

void HyperliquidGWApplication::onPostResponse(const std::string& message, hyperliquid::RestEndpointType type)
{
    m_restParser.parse(message, type);
}

void HyperliquidGWApplication::onConnected()
{
    spdlog::info("HyperliquidGW: Connected");
    m_connected = true;
    m_websocket->subscribe(hyperliquid::SubscriptionType::OrderUpdates);
    m_websocket->subscribe(hyperliquid::SubscriptionType::UserFills);
}

void HyperliquidGWApplication::onDisconnected(bool hasError, const std::string& errMsg)
{
    spdlog::info("HyperliquidGW: Disconnected{}", hasError ? " (error: " + errMsg + ")" : "");
    m_connected = false;
}

// WebsocketMessageHandler

void HyperliquidGWApplication::onOrderUpdate(const hyperliquid::OrderUpdate& update)
{
    spdlog::info("HyperliquidGW: OrderUpdate coin={} side={} status={} oid={} sz={} limitPx={} cloid={}",
                 update.coin, update.side, hyperliquid::toString(update.status),
                 update.oid, update.sz, update.limitPx, update.cloid);

    com::liversedge::messages::ExecutionReport sbeExecReport;
    if (!m_sbeWriter.prepareMessage(sbeExecReport))
    {
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

    sbeExecReport.timestamp(timestamp);
    sbeExecReport.transactTime(update.statusTimestamp * 1000000ULL); // ms -> ns
    sbeExecReport.securityId(m_refDataHolder.getSecurityIdBySymbol(update.coin));
    sbeExecReport.ordStatus(mapOrderStatus(update.status));
    sbeExecReport.side(mapSide(update.side));
    sbeExecReport.orderType(com::liversedge::messages::OrderType::LIMIT);
    sbeExecReport.ordRejReason(
        (update.status == hyperliquid::OrderStatus::Rejected || update.status == hyperliquid::OrderStatus::OracleRejected)
            ? com::liversedge::messages::OrdRejReason::OTHER
            : com::liversedge::messages::OrdRejReason::NO_REJECT);

    SBEUtils::setPrice(sbeExecReport.price(), std::to_string(update.limitPx));
    SBEUtils::setQty(sbeExecReport.orderQty(), std::to_string(update.origSz));
    SBEUtils::setQty(sbeExecReport.leavesQty(), std::to_string(update.sz));

    double cumQty = update.origSz - update.sz;
    SBEUtils::setQty(sbeExecReport.cumQty(), std::to_string(cumQty));

    SBEUtils::setVarString(sbeExecReport, sbeExecReport.origClientOrderId(), update.cloid);
    SBEUtils::setVarString(sbeExecReport, sbeExecReport.clientOrderId(), update.cloid);
    SBEUtils::setVarString(sbeExecReport, sbeExecReport.text(), "");

    m_sbeWriter.writeMessage(sbeExecReport);
    spdlog::info("HyperliquidGW: Sent SBE ExecutionReport for order update");
}

void HyperliquidGWApplication::onUserFill(const hyperliquid::Fill& fill)
{
    spdlog::info("HyperliquidGW: Fill coin={} side={} px={} sz={} oid={}",
                 fill.coin, fill.side, fill.px, fill.sz, fill.oid);

    com::liversedge::messages::ExecutionReport sbeExecReport;
    if (!m_sbeWriter.prepareMessage(sbeExecReport))
    {
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

    sbeExecReport.timestamp(timestamp);
    sbeExecReport.transactTime(fill.time * 1000000ULL); // ms -> ns
    sbeExecReport.securityId(m_refDataHolder.getSecurityIdBySymbol(fill.coin));
    sbeExecReport.ordStatus(com::liversedge::messages::OrdStatus::PARTIALLY_FILLED);
    sbeExecReport.side(mapSide(fill.side));
    sbeExecReport.orderType(com::liversedge::messages::OrderType::LIMIT);
    sbeExecReport.ordRejReason(com::liversedge::messages::OrdRejReason::NO_REJECT);

    SBEUtils::setPrice(sbeExecReport.lastPx(), std::to_string(fill.px));
    SBEUtils::setQty(sbeExecReport.lastQty(), std::to_string(fill.sz));

    SBEUtils::setVarString(sbeExecReport, sbeExecReport.origClientOrderId(), "");
    SBEUtils::setVarString(sbeExecReport, sbeExecReport.clientOrderId(), "");
    SBEUtils::setVarString(sbeExecReport, sbeExecReport.text(), "");

    m_sbeWriter.writeMessage(sbeExecReport);
    spdlog::info("HyperliquidGW: Sent SBE Fill ExecutionReport");
}

// RestEndpointListener

void HyperliquidGWApplication::onPlaceOrder(const hyperliquid::PlaceOrderResponse& response)
{
    spdlog::info("HyperliquidGW: PlaceOrder response status={}", response.status);

    if (response.status != "ok")
    {
        for (const auto& s : response.statuses)
        {
            if (s.error)
            {
                spdlog::error("HyperliquidGW: PlaceOrder error: {}", *s.error);
            }
        }
    }
}

void HyperliquidGWApplication::onModifyOrder(const hyperliquid::ModifyOrderResponse& response)
{
    spdlog::info("HyperliquidGW: ModifyOrder response status={}", response.status);
}

void HyperliquidGWApplication::onCancelOrder(const hyperliquid::CancelOrderResponse& response)
{
    spdlog::info("HyperliquidGW: CancelOrder response status={}", response.status);

    if (response.status != "ok")
    {
        for (const auto& s : response.statuses)
        {
            if (s.error)
            {
                spdlog::error("HyperliquidGW: CancelOrder error: {}", *s.error);
            }
        }
    }
}

// Private helpers

hyperliquid::Environment HyperliquidGWApplication::getEnvironment(const std::string& envName)
{
    if (envName == "prod")
    {
        return hyperliquid::Environment::Mainnet;
    }
    else if (envName == "testnet")
    {
        return hyperliquid::Environment::Testnet;
    }
    else
    {
        throw std::runtime_error("Unrecognized environment: " + envName);
    }
}

com::liversedge::messages::OrdStatus::Value HyperliquidGWApplication::mapOrderStatus(hyperliquid::OrderStatus status)
{
    switch (status)
    {
    case hyperliquid::OrderStatus::Open:
    case hyperliquid::OrderStatus::Triggered:
        return com::liversedge::messages::OrdStatus::NEW;
    case hyperliquid::OrderStatus::Filled:
        return com::liversedge::messages::OrdStatus::FILLED;
    case hyperliquid::OrderStatus::Canceled:
    case hyperliquid::OrderStatus::MarginCanceled:
        return com::liversedge::messages::OrdStatus::CANCELLED;
    case hyperliquid::OrderStatus::Rejected:
    case hyperliquid::OrderStatus::OracleRejected:
        return com::liversedge::messages::OrdStatus::REJECTED;
    default:
        return com::liversedge::messages::OrdStatus::NEW;
    }
}

com::liversedge::messages::Side::Value HyperliquidGWApplication::mapSide(char side)
{
    return (side == 'B') ? com::liversedge::messages::Side::BUY : com::liversedge::messages::Side::SELL;
}
