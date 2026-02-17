#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include "../util/SimpleConfig.h"
#include "hyperliquid/MessageHandler.h"
#include "hyperliquid/SocketListener.h"

using encoding_t = unsigned char const*;

class HyperliquidMDApplicationBase : public hyperliquid::MessageHandler, public hyperliquid::SocketListener
{
public:
    HyperliquidMDApplicationBase(const SimpleConfig& config) : m_config{ config } {}

    // hyperliquid::SocketListener
    void onMessage(const std::string& message) override;
    void onConnected() override;
    void onDisconnected() override;

    // hyperliquid::MessageHandler
    void onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level) override;
    void onTrade(const hyperliquid::Trade& trade) override;

private:
    const SimpleConfig& m_config;
};

#endif