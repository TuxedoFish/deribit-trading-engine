#pragma once

#include "../util/SimpleConfig.h"
#include "MarketDataLogger.h"
#include <memory>
#include <string>

#include "../marketdata/HyperliquidMDApplicationBase.h"
#include "hyperliquid/SocketListener.h"

class HyperliquidPersister : public HyperliquidMDApplicationBase
{
    std::unique_ptr<MarketDataLogger> m_logger;

public:
    // Constructor with optional log directory
    explicit HyperliquidPersister(SimpleConfig& config);
    virtual ~HyperliquidPersister() = default;

    // hyperliquid::SocketListener
    void onMessage(const std::string& message) override;
    void onConnected() override;
    void onDisconnected() override;
};
