#include "../../include/marketdata/HyperliquidMDApplicationBase.h"


// hyperliquid::SocketListener
void HyperliquidMDApplicationBase::onMessage(const std::string& message) override {
}

void HyperliquidMDApplicationBase::onConnected() override {
}

void HyperliquidMDApplicationBase::onDisconnected() override {
}

// hyperliquid::MessageHandler
void HyperliquidMDApplicationBase::onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level) override {
}

void HyperliquidMDApplicationBase::onTrade(const hyperliquid::Trade& trade) override {
}
