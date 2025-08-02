// main.cpp : Defines the entry point for the application.

#include "main.h"

int main()
{
    try
    {
        // Load configuration
        FIX::SessionSettings settings("deribit-md/config/deribit.cfg");

        // Create application
        ApplicationPersister application("C:/Users/harry/Documents/Data/deribit-raw-capture");

        // Create stores and logs
        FIX::FileStoreFactory storeFactory(settings);
        FIX::FileLogFactory logFactory(settings);

        // Create initiator
        FIX::ThreadedSSLSocketInitiator initiator(application, storeFactory, settings, logFactory);

        // Start the connection
        initiator.start();
        std::cout << "FIX client started. Type #quit to quit..." << std::endl;

        while (true) {
            std::string value;
            std::cin >> value;

            if (value == "#quit") {
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