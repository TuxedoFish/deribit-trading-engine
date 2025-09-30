#include "../include/main.h"

int main(int argc, char* argv[])
{
    CmdLineOptions options(argc, argv);
    std::string applicationName = "UNSET";
    if (options.cmdOptionExists("--app"))
    {
        applicationName = options.getCmdOption("--app");
    }

    if (applicationName == "md-hist") {
        std::cout << "Running as: " << applicationName << std::endl;
        SimpleConfig config("config/settings.md-hist.txt");
        AppRunner app(config);
        return app.runMarketdataHistoricalStorage();
    }
    if (applicationName == "md-process") {
        std::cout << "Running as: " << applicationName << std::endl;
        SimpleConfig config("config/settings.md-process.txt");
        AppRunner app(config);
        return app.runProcessRawMarketdata();
    }
    if (applicationName.rfind("gw-testnet", 0) == 0) {
        std::cout << "Running as: " << applicationName << std::endl;
        SimpleConfig config("config/settings.gw-testnet.txt");
        AppRunner app(config);
        return app.runGateway();
    }
    if (applicationName.rfind("gw-prod", 0) == 0) {
        // TODO: Not safe to trial this
        // std::cout << "Running as: " << applicationName << std::endl;
        // SimpleConfig config("config/settings.gw-prod.txt");
        // AppRunner app(config);
        // return app.runMarketdata();
    }
    if (applicationName == "md-prod") {
        std::cout << "Running as: " << applicationName << std::endl;
        SimpleConfig config("config/settings.md-prod.txt");
        AppRunner app(config);
        return app.runMarketdata();
    }
    if (applicationName.rfind("md-prod-to-testnet", 0) == 0) {
        std::cout << "Running as: " << applicationName << std::endl;
        SimpleConfig config("config/settings.md-prod-to-testnet.txt");
        AppRunner app(config);
        return app.runMarketdata();
    }
    if (applicationName.rfind("md-testnet", 0) == 0) {
        std::cout << "Running as: " << applicationName << std::endl;
        SimpleConfig config("config/settings.md-testnet.txt");
        AppRunner app(config);
        return app.runMarketdata();
    }
    std::cout << "Unknown application: " << applicationName << std::endl;
    return 0;
}

