#pragma once

#include "MessageProcessor.h"
#include "hyperliquid/types/ResponseTypes.h"

class HyperliquidMessageProcessor : public MessageProcessor
{
public:
    explicit HyperliquidMessageProcessor(SBEBinaryWriter& writer);
    ~HyperliquidMessageProcessor() = default;

    void onConnected();
    void onDisconnected(bool hasError, const std::string& errMsg);
    void onMeta(const hyperliquid::MetaResponse& response);
    void onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level);
    void onTrade(const hyperliquid::Trade& trade);

    void setDesiredCoins(const std::set<std::string>& desiredCoins);
private:
    std::set<std::string> m_desiredCoins;
    std::set<std::string> m_observedCoins;
    uint64_t m_connectedTimeMs{0};
};
