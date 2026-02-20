//
// Created by markl on 25/09/2025.
//

#include "../../include/marketdata/HyperliquidMDApplication.h"

HyperliquidMDApplication::HyperliquidMDApplication(const SimpleConfig& config)
    : HyperliquidMDApplicationBase(config), m_processor(m_writer)
{
    m_writer.openNewFile(config.getString("md_file_path") + kPathSeparator + "messages.sbe");
}

// hyperliquid::WebsocketListener
void HyperliquidMDApplication::onConnected() {
    m_processor.onConnected();
    HyperliquidMDApplicationBase::onConnected();
}

void HyperliquidMDApplication::onDisconnected() {
    m_processor.onDisconnected();
    HyperliquidMDApplicationBase::onDisconnected();
}

// hyperliquid::InfoEndpointListener
void HyperliquidMDApplication::onMeta(const hyperliquid::MetaResponse& response) {
    m_processor.onMeta(response);
    HyperliquidMDApplicationBase::onMeta(response);
}

// hyperliquid::WSMessageHandler
void HyperliquidMDApplication::onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level) {
    m_processor.onL2BookLevel(book, level);
}

void HyperliquidMDApplication::onTrade(const hyperliquid::Trade& trade) {
    m_processor.onTrade(trade);
}
