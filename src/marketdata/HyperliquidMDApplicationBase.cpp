#include "../../include/marketdata/HyperliquidMDApplicationBase.h"
#include "hyperliquid/websocket/MarketData.h"
#include "hyperliquid/rest/InfoApi.h"
#include "hyperliquid/rest/RestMessageParser.h"
#include <iostream>
#include <set>

HyperliquidMDApplicationBase::~HyperliquidMDApplicationBase() = default;

void HyperliquidMDApplicationBase::start()
{
    // Convert the CSV of desired coins to a set of coins
    std::stringstream desiredCoinsCsv(m_config.getString("coins", "BTC"));
    std::string coin;
    while(std::getline(desiredCoinsCsv, coin, ','))
    {
        m_desiredCoins.insert(coin);
    }

    // Start info API (RAII — thread starts in constructor)
    m_infoApi = std::make_unique<hyperliquid::InfoApi>(hyperliquid::Environment::Mainnet, *this);

    // Start market data websocket (reconnection with backoff is handled by the SDK)
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
    // Once connected query for securities to start subscribing to
    m_infoApi->sendRequest(hyperliquid::InfoEndpointType::Meta); // Main DEX
    m_infoApi->sendRequest(hyperliquid::InfoEndpointType::Meta, {{"dex", "xyz"}}); // XYZ DEX (Tokenized equities)
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
        if (m_desiredCoins.find(coin.name) != m_desiredCoins.end())
        {
            std::cout << "Subscribing to " << coin.name << std::endl;
            subscribeToMarket(coin.name);
        }
    }
}

// hyperliquid::WSMessageHandler
void HyperliquidMDApplicationBase::onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level) {
}

void HyperliquidMDApplicationBase::onTrade(const hyperliquid::Trade& trade) {
}
