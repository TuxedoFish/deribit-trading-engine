#include "../../include/historical/HyperliquidPersister.h"

HyperliquidPersister::HyperliquidPersister(SimpleConfig& config) : HyperliquidMDApplicationBase(config),
    m_logger(std::make_unique<MarketDataLogger>(config.getString("md_raw_ws_file_path")))
{
}

// hyperliquid::WebsocketListener
void HyperliquidPersister::onMessage(const std::string& message)
{
    if (m_logger)
    {
        m_logger->writeToLog("IN_APP", message);
    }
}

void HyperliquidPersister::onConnected()
{
    std::cout << "Connected" << std::endl;
}

void HyperliquidPersister::onDisconnected()
{
    std::cout << "Disconnected" << std::endl;
}

// hyperliquid::RestListener
void HyperliquidPersister::onMessage(const std::string& message, hyperliquid::InfoEndpointType type)
{
    if (m_logger)
    {
        m_logger->writeToLog("IN_APP", message);
    }
    HyperliquidMDApplicationBase::onMessage(message, type);
}
