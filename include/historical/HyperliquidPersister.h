#pragma once

#include "../util/SimpleConfig.h"
#include "MarketDataLogger.h"
#include "../marketdata/HyperliquidMDApplicationBase.h"
#include <memory>
#include <string>

class HyperliquidPersister : public HyperliquidMDApplicationBase {
private:
    std::unique_ptr<MarketDataLogger> m_logger;

public:
    // Constructor with optional log directory
    explicit HyperliquidPersister(SimpleConfig& config);
    virtual ~HyperliquidPersister() = default;

    // hyperliquid::WebsocketListener
    void onMessage(const std::string& message) override;
    void onConnected() override;
    void onDisconnected() override;

    // hyperliquid::RestListener
    virtual void onMessage(const std::string& message, hyperliquid::InfoEndpointType type) override;
};
