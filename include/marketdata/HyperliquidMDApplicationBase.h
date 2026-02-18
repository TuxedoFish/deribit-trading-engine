#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../util/SimpleConfig.h"
#include "hyperliquid/rest/InfoApi.h"
#include "hyperliquid/websocket/WSMessageHandler.h"
#include "hyperliquid/websocket/WebsocketListener.h"
#include "hyperliquid/rest/InfoEndpointListener.h"
#include "hyperliquid/rest/RestListener.h"
#include "hyperliquid/websocket/MarketData.h"

class HyperliquidMDApplicationBase : public hyperliquid::WSMessageHandler,
                                     public hyperliquid::WebsocketListener,
                                     public hyperliquid::RestListener,
                                     public hyperliquid::InfoEndpointListener
{
public:
    HyperliquidMDApplicationBase(const SimpleConfig& config) : m_config{ config } {}
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

    // hyperliquid::WSMessageHandler
    virtual void onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level) override;
    virtual void onTrade(const hyperliquid::Trade& trade) override;

    // hyperliquid::InfoEndpointListener
    virtual void onMeta(const hyperliquid::MetaResponse& response) override;

protected:
    virtual void subscribeToMarket(const std::string& coin);
    const SimpleConfig& m_config;
    std::vector<hyperliquid::AssetMeta> m_universe;

private:
    std::unique_ptr<hyperliquid::MarketData> m_marketData;
    std::unique_ptr<hyperliquid::InfoApi> m_infoApi;
};
