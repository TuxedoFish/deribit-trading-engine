#include "../include/AppRunner.h"
#include "../include/gateway/RefDataHolder.h"

AppRunner::AppRunner(const SimpleConfig& config) : config_{ config } {
}

int AppRunner::runMarketdata()
{
    // Create application
    MDApplication application(config_);

    // Create FIX runner and start session
    FIXRunner fixRunner(config_);
    std::string startupMessage = "Publishing messages to: " + config_.getString("md_file_path");

    return fixRunner.run(application, startupMessage);
}

int AppRunner::runGateway()
{
    // Create RefDataHolder
    RefDataHolder refDataHolder;

    // Create application
    GWApplication application(config_, refDataHolder);

    // Create FIX runner and gateway runner (passing application reference)
    FIXRunner fixRunner(config_);
    GWRunner gatewayRunner(config_, application, refDataHolder);
    std::string startupMessage = "Publishing inbound executions to: " + config_.getString("gw_inbound_file_path");

    return fixRunner.run(application, startupMessage, [&gatewayRunner]() { gatewayRunner.run(); }, true);
}

int AppRunner::runProcessRawMarketdata() {
    MarketdataHistoricalRunner runner(config_);
    return runner.run();
}

int AppRunner::runMarketdataHistoricalStorage() {
    // Create application
    ApplicationPersister application(config_);

    // Create FIX runner and start session
    FIXRunner fixRunner(config_);
    std::string startupMessage = "Publishing raw FIX messages to: " + config_.getString("md_raw_fix_file_path");

    return fixRunner.run(application, startupMessage);
}