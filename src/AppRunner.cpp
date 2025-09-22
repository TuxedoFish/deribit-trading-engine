#include "../include/AppRunner.h"

AppRunner::AppRunner(const SimpleConfig& config) : config_{ config } {
}


int AppRunner::runProcessRawMarketdata() {
    MarketdataHistoricalRunner runner(config_);
    return runner.run();
}

int AppRunner::runMarketdataHistoricalStorage() {
    try
    {
        // Load configuration
        FIX::SessionSettings settings(config_.getString("fix_settings_file_path"));

        // Create application
        std::cout << "Publishing raw FIX messages to: " << config_.getString("md_raw_fix_file_path") << std::endl;
        ApplicationPersister application(config_);

        // Create stores and logs
        FIX::FileStoreFactory storeFactory(settings);
        NullLogFactory logFactory; // TODO: Use FileLogFactory(settings) for trading

        // Create initiator
        FIX::ThreadedSSLSocketInitiator initiator(application, storeFactory, settings, logFactory);

        // Start the connection
        initiator.start();
        std::cout << "FIX client started. Type q to quit..." << std::endl;

        while (true) {
            std::string value;
            std::cin >> value;

            if (value == "q") {
                break;
            }

            std::cout << std::endl;
        }

        // Stop the connection
        initiator.stop();

    }
    catch (std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}