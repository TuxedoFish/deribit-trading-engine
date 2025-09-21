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
    else if (applicationName == "md-process") {
        std::cout << "Running as: " << applicationName << std::endl;
        SimpleConfig config("config/settings.md-process.txt");
        AppRunner app(config);
        return app.runProcessRawMarketdata();
    }
    else {
        std::cout << "Unknown application: " << applicationName << std::endl;
        return 0;
    }
}

