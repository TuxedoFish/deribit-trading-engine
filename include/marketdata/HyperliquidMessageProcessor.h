#pragma once

#include "MessageProcessor.h"
#include "hyperliquid/types/InfoEndpointTypes.h"
#include "hyperliquid/types/ResponseTypes.h"

class HyperliquidMessageProcessor : public MessageProcessor
{
public:
    explicit HyperliquidMessageProcessor(SBEBinaryWriter& writer);
    ~HyperliquidMessageProcessor() = default;

    void onConnected();
    void onDisconnected();
    void onMeta(const hyperliquid::MetaResponse& response);
    void onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level);
    void onTrade(const hyperliquid::Trade& trade);
};
