#include "../../include/marketdata/HyperliquidMDApplicationBase.h"
#include "hyperliquid/websocket/MarketData.h"
#include "hyperliquid/rest/InfoApi.h"
#include "hyperliquid/rest/RestMessageParser.h"
#include <iostream>

HyperliquidMDApplicationBase::~HyperliquidMDApplicationBase() = default;

void HyperliquidMDApplicationBase::start()
{
    // Start info API (RAII — thread starts in constructor)
    m_infoApi = std::make_unique<hyperliquid::InfoApi>(hyperliquid::Environment::Mainnet, *this);

    // Fetch security list — response arrives via onMessage(string, InfoEndpointType)
    m_infoApi->sendRequest(hyperliquid::InfoEndpointType::Meta); // Main DEX
    m_infoApi->sendRequest(hyperliquid::InfoEndpointType::Meta, {{"dex", "xyz"}}); // XYZ DEX (Tokenized equities)

    // Start market data websocket
    m_marketData = std::make_unique<hyperliquid::MarketData>(hyperliquid::Environment::Mainnet, *this);
    m_marketData->start();
}

void HyperliquidMDApplicationBase::stop()
{
    if (m_marketData)
    {
        m_marketData->stop();
    }
    m_infoApi.reset();
}

void HyperliquidMDApplicationBase::subscribeToMarket(const std::string& coin)
{
    m_marketData->subscribe(hyperliquid::SubscriptionType::L2Book, {{"coin", coin}});
    m_marketData->subscribe(hyperliquid::SubscriptionType::Trades, {{"coin", coin}});
}

// hyperliquid::WebsocketListener
void HyperliquidMDApplicationBase::onMessage(const std::string& message) {
}

void HyperliquidMDApplicationBase::onConnected() {
}

void HyperliquidMDApplicationBase::onDisconnected() {
}

// hyperliquid::RestListener — parse and dispatch to typed callbacks
void HyperliquidMDApplicationBase::onMessage(const std::string& message, hyperliquid::InfoEndpointType type) {
    hyperliquid::RestMessageParser parser(*this);
    parser.parse(message, type);
}

// hyperliquid::InfoEndpointListener
void HyperliquidMDApplicationBase::onMeta(const hyperliquid::MetaResponse& response) {
    m_universe = response.universe;
    std::cout << "Loaded " << m_universe.size() << " assets" << std::endl;

    // Now start WS and subscribe
    for (const auto& coin : m_universe)
    {
        subscribeToMarket(coin.name);
    }
}

// hyperliquid::WSMessageHandler
void HyperliquidMDApplicationBase::onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level) {
}

void HyperliquidMDApplicationBase::onTrade(const hyperliquid::Trade& trade) {
}
