#include "../include/AppRunner.h"

AppRunner::AppRunner(const SimpleConfig& config) : config_{ config } {
}

int AppRunner::runMarketdata()
{
    // Create application
    ApplicationWrapper application(config_);

    // Create FIX runner and start session
    FIXRunner fixRunner(config_);
    std::string startupMessage = "Publishing messages to: " + config_.getString("md_file_path");

    return fixRunner.run(application, startupMessage);
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