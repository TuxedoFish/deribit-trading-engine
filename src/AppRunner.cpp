#include "../include/AppRunner.h"
#include "../include/gateway/RefDataHolder.h"
#include "../include/marketdata/HyperliquidMDApplication.h"
#include "../include/sbe/SBEBinaryWriter.h"
#include "../include/util/ConsoleUtils.h"
#include "../include/util/SimpleConfig.h"

AppRunner::AppRunner(SimpleConfig& config) : config_{config}
{
}

int AppRunner::runMarketdata()
{
    std::string exchangeName = (config_.getString("exchange_name", "UNSET"));

    if (exchangeName == "deribit") {
        // Create application
        DeribitApplication application(config_);

        // Create FIX runner and start session
        FIXRunner fixRunner(config_);
        std::string startupMessage = "Publishing messages to: " + config_.getString("md_file_path");

        return fixRunner.run(application, startupMessage);
    }
    if (exchangeName == "hyperliquid")
    {
        HyperliquidMDApplication application(config_);
        application.start();

        std::cout << "Publishing messages to: " + config_.getString("md_file_path") << std::endl;
        ConsoleUtils::waitForUserInput();
        return 1;
    }

    std::cout << "Unrecognized exchange type: " << exchangeName << std::endl;
    return 1;
}

int AppRunner::runGateway()
{
    // Create RefDataHolder
    RefDataHolder refDataHolder;

    // Create SBE writer
    SBEBinaryWriter sbeWriter;
    sbeWriter.openNewFile(config_.getString("gw_outbound_file_path") + kPathSeparator + "messages.sbe", true);

    std::string exchangeName = config_.getString("exchange_name", "UNSET");

    if (exchangeName == "deribit")
    {
        // Create application
        DeribitGWApplication application(config_, refDataHolder, sbeWriter);

        // Create FIX runner and gateway runner (passing application reference)
        FIXRunner fixRunner(config_);
        DeribitOrdersHandler ordersHandler(refDataHolder, application, sbeWriter);
        GWRunner gatewayRunner(config_, ordersHandler, refDataHolder, sbeWriter);
        std::string startupMessage = "Publishing inbound executions to: " + config_.getString("gw_inbound_file_path");

        return fixRunner.run(application, startupMessage, [&gatewayRunner]() { gatewayRunner.run(); }, true);
    }
    if (exchangeName == "hyperliquid")
    {
        HyperliquidOrdersHandler ordersHandler(refDataHolder, sbeWriter);
        GWRunner gatewayRunner(config_, ordersHandler, refDataHolder, sbeWriter);
        std::string startupMessage = "Publishing inbound executions to: " + config_.getString("gw_inbound_file_path");
        gatewayRunner.run();
        ConsoleUtils::waitForUserInput();
        return 1;
    }

    std::cout << "Unrecognized exchange type: " << exchangeName << std::endl;
    return 1;
}

int AppRunner::runProcessRawMarketdata()
{
    MarketdataHistoricalRunner runner(config_);
    return runner.run();
}

int AppRunner::runMarketdataHistoricalStorage()
{
    std::string exchangeName = config_.getString("exchange_name", "UNSET");

    if (exchangeName == "deribit") {
        // Create application
        DeribitPersister application(config_);

        // Create FIX runner and start session
        FIXRunner fixRunner(config_);
        std::string startupMessage = "Publishing raw FIX messages to: " + config_.getString("md_raw_file_path");

        return fixRunner.run(application, startupMessage);
    }
    if (exchangeName == "hyperliquid")
    {
        HyperliquidPersister application(config_);
        application.start();

        std::cout << "Publishing raw WS messages to: " + config_.getString("md_raw_file_path") << std::endl;
        ConsoleUtils::waitForUserInput();
        application.stop();
        return 1;
    }

    std::cout << "Unrecognized exchange type: " << exchangeName << std::endl;
    return 1;
}
