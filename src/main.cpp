#include "../include/main.h"

int main(int argc, char* argv[])
{
    CmdLineOptions options(argc, argv);
    std::string applicationName = "UNSET";
    if (options.cmdOptionExists("--app"))
    {
        applicationName = options.getCmdOption("--app");
    }
    // Defaults to e.g. settings.md-process.txt
    std::string configName = applicationName;
    if (options.cmdOptionExists("--config-override"))
    {
        configName = options.getCmdOption("--config-override");
    }

    // Marketdata
    if (applicationName.rfind("md-hist") != std::string::npos) {
        std::cout << "Running as md-hist: " << applicationName << std::endl;
        SimpleConfig config("config/settings." + configName + ".txt");
        AppRunner app(config);
        return app.runMarketdataHistoricalStorage();
    }
    if (applicationName.rfind("md-process") != std::string::npos) {
        std::cout << "Running as md-process: " << applicationName << std::endl;
        SimpleConfig config("config/settings." + configName + ".txt");
        AppRunner app(config);
        return app.runProcessRawMarketdata();
    }
    if (applicationName.rfind("md-") != std::string::npos) {
        std::cout << "Running as md: " << applicationName << std::endl;
        SimpleConfig config("config/settings." + configName + ".txt");
        AppRunner app(config);
        return app.runMarketdata();
    }

    // Gateway
    if (applicationName.rfind("gw-testnet", 0)  != std::string::npos) {
        std::cout << "Running as gw: " << applicationName << std::endl;
        SimpleConfig config("config/settings." + configName + ".txt");
        AppRunner app(config);
        return app.runGateway();
    }
    if (applicationName.rfind("gw-prod", 0) != std::string::npos) {
        std::cout << "Running as gw: " << applicationName << std::endl;
        SimpleConfig config("config/settings." + configName + ".txt");
        AppRunner app(config);
        return app.runGateway();
    }
    std::cout << "Unknown application: " << applicationName << std::endl;
    return 0;
}

