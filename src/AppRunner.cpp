#include "../include/AppRunner.h"
#include "../include/gateway/RefDataHolder.h"
#include "../include/sbe/SBEBinaryWriter.h"
#include "../include/util/SimpleConfig.h"
#include "hyperliquid/MarketData.h"
#include "src/internal/WSRunner.h"

AppRunner::AppRunner(SimpleConfig& config) : config_{config}
{
}

int AppRunner::runMarketdata()
{
    // Create application
    DeribitApplication application(config_);

    // Create FIX runner and start session
    FIXRunner fixRunner(config_);
    std::string startupMessage = "Publishing messages to: " + config_.getString("md_file_path");

    return fixRunner.run(application, startupMessage);
}

int AppRunner::runGateway()
{
    // Create RefDataHolder
    RefDataHolder refDataHolder;

    // Create SBE writer
    SBEBinaryWriter sbeWriter;
    sbeWriter.openNewFile(config_.getString("gw_outbound_file_path") + kPathSeparator + "messages.sbe", true);

    // Create application
    GWApplication application(config_, refDataHolder, sbeWriter);

    // Create FIX runner and gateway runner (passing application reference)
    FIXRunner fixRunner(config_);
    GWRunner gatewayRunner(config_, application, refDataHolder, sbeWriter);
    std::string startupMessage = "Publishing inbound executions to: " + config_.getString("gw_inbound_file_path");

    return fixRunner.run(application, startupMessage, [&gatewayRunner]() { gatewayRunner.run(); }, true);
}

int AppRunner::runProcessRawMarketdata()
{
    MarketdataHistoricalRunner runner(config_);
    return runner.run();
}

int AppRunner::runMarketdataHistoricalStorage()
{
    std::string exchangeName = (config_.getString("exchange_name", "UNSET"));

    if (exchangeName == "deribit") {
        // Create application
        DeribitPersister application(config_);

        // Create FIX runner and start session
        FIXRunner fixRunner(config_);
        std::string startupMessage = "Publishing raw FIX messages to: " + config_.getString("md_raw_fix_file_path");

        return fixRunner.run(application, startupMessage);
    }
    if (exchangeName == "hyperliquid")
    {
        // Create application
        HyperliquidPersister application(config_);

        // Create WS runner and start up stream
        hyperliquid::MarketData marketdata(hyperliquid::Environment::Mainnet, application);
        marketdata.start();
        marketdata.subscribe(hyperliquid::MessageType::L2Book, {{"coin", "BTC"}});
        marketdata.subscribe(hyperliquid::MessageType::Trades, {{"coin", "BTC"}});

        std::cout << "Publishing raw WS messages to: " + config_.getString("md_raw_ws_file_path") << std::endl;
        while (true)
        {
            std::string value;
            std::cin >> value;

            if (value == "q")
            {
                break;
            }

            std::cout << std::endl;
        }

        marketdata.stop();
    }

    std::cout << "Unrecognized exchange type: " << exchangeName << std::endl;
    return 1;
}
