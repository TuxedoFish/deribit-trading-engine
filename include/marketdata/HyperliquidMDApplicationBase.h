#pragma once

#include <atomic>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "../util/SimpleConfig.h"
#include "hyperliquid/rest/InfoApi.h"
#include "hyperliquid/websocket/WSMessageHandler.h"
#include "hyperliquid/websocket/WebsocketListener.h"
#include "hyperliquid/rest/InfoEndpointListener.h"
#include "hyperliquid/rest/RestListener.h"
#include "hyperliquid/rest/RestMessageParser.h"
#include "hyperliquid/websocket/WSMessageParser.h"
#include "hyperliquid/websocket/MarketData.h"

class HyperliquidMDApplicationBase : public hyperliquid::WebsocketListener,
                                     public hyperliquid::RestListener,
                                     public hyperliquid::InfoEndpointListener,
                                     public hyperliquid::WSMessageHandler
{
public:
    HyperliquidMDApplicationBase(const SimpleConfig& config) : m_config{ config }, m_restParser(*this) {}
    virtual ~HyperliquidMDApplicationBase();

    // Lifecycle
    void start();
    void stop();

    // hyperliquid::WebsocketListener
    virtual void onMessage(const std::string& message) override;
    virtual void onConnected() override;
    virtual void onDisconnected() override;

    // hyperliquid::RestListener
    virtual void onMessage(const std::string& message, hyperliquid::InfoEndpointType type) override;

    // hyperliquid::InfoEndpointListener
    virtual void onMeta(const hyperliquid::MetaResponse& response) override;

protected:
    virtual void subscribeToMarket(const std::string& coin);
    const SimpleConfig& m_config;
    std::set<std::string> m_desiredCoins;
    std::vector<hyperliquid::AssetMeta> m_universe;

private:
    std::unique_ptr<hyperliquid::MarketData> m_marketData;
    std::unique_ptr<hyperliquid::InfoApi> m_infoApi;
    hyperliquid::RestMessageParser m_restParser;
    hyperliquid::WSMessageParser m_wsParser;
};
