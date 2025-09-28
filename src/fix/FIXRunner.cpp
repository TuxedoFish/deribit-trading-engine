#include "../../include/fix/FIXRunner.h"

FIXRunner::FIXRunner(const SimpleConfig& config) : config_(config)
{
}

int FIXRunner::run(FIX::Application& application, const std::string& startupMessage)
{
    return run(application, startupMessage, waitForUserInput, false);
}

int FIXRunner::run(FIX::Application& application, const std::string& startupMessage, std::function<void()> mainLoop,
                   bool logFixMessages)
{
    try
    {
        // Load configuration
        FIX::SessionSettings settings(config_.getString("fix_settings_file_path"));

        std::cout << startupMessage << std::endl;

        // Create stores and logs
        FIX::FileStoreFactory storeFactory(settings);
        if (logFixMessages)
        {
            FIX::FileLogFactory logFactory(settings);
            FIX::ThreadedSSLSocketInitiator initiator(application, storeFactory, settings, logFactory);
            runWithInitiator(mainLoop, initiator);
        }
        else
        {
            NullLogFactory logFactory;
            FIX::ThreadedSSLSocketInitiator initiator(application, storeFactory, settings, logFactory);
            runWithInitiator(mainLoop, initiator);
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void FIXRunner::runWithInitiator(std::function<void()> mainLoop, FIX::ThreadedSSLSocketInitiator& initiator)
{
    // Start the connection
    initiator.start();
    std::cout << "FIX client started. Type q to quit..." << std::endl;

    // Run the main loop
    mainLoop();

    // Stop the connection
    initiator.stop();
}

void FIXRunner::waitForUserInput()
{
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
}
