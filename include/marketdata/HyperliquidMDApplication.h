#pragma once

#include "HyperliquidMDApplicationBase.h"
#include "HyperliquidMessageProcessor.h"

class HyperliquidMDApplication : public HyperliquidMDApplicationBase
{
public:
    explicit HyperliquidMDApplication(const SimpleConfig& config);
    virtual ~HyperliquidMDApplication() = default;

    // hyperliquid::WebsocketListener
    void onConnected() override;
    void onDisconnected() override;

    // hyperliquid::InfoEndpointListener
    void onMeta(const hyperliquid::MetaResponse& response) override;

    // hyperliquid::WSMessageHandler
    void onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level) override;
    void onTrade(const hyperliquid::Trade& trade) override;

private:
    SBEBinaryWriter m_writer;
    HyperliquidMessageProcessor m_processor;
};
