#include "../../include/fix/FIXRunner.h"

FIXRunner::FIXRunner(const SimpleConfig& config) : config_(config) {
}

int FIXRunner::run(FIX::Application& application, const std::string& startupMessage) {
    try {
        // Load configuration
        FIX::SessionSettings settings(config_.getString("fix_settings_file_path"));

        std::cout << startupMessage << std::endl;

        // Create stores and logs
        FIX::FileStoreFactory storeFactory(settings);
        NullLogFactory logFactory; // TODO: Use FileLogFactory(settings) for trading

        // Create initiator
        FIX::ThreadedSSLSocketInitiator initiator(application, storeFactory, settings, logFactory);

        // Start the connection
        initiator.start();
        std::cout << "FIX client started. Type q to quit..." << std::endl;

        // Wait for user input
        waitForUserInput();

        // Stop the connection
        initiator.stop();

    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void FIXRunner::waitForUserInput() {
    while (true) {
        std::string value;
        std::cin >> value;

        if (value == "q") {
            break;
        }

        std::cout << std::endl;
    }
}