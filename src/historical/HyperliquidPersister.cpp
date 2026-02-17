#include "../../include/historical/HyperliquidPersister.h"

HyperliquidPersister::HyperliquidPersister(SimpleConfig& config) : HyperliquidMDApplicationBase(config),
    m_logger(std::make_unique<MarketDataLogger>(config.getString("md_raw_ws_file_path")))
{
}

void HyperliquidPersister::onMessage(const std::string& message)
{
    std::cout << message << std::endl;
    if (!m_logger)
    {
        m_logger.get()->writeToLog("IN_APP", message);
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
