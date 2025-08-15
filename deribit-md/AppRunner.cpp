#include "AppRunner.h"

AppRunner::AppRunner(const SimpleConfig& config) : config_{ config } {
}

std::string getMonthDayString(int dayOrMonth) {
    if (dayOrMonth > 9) {
        return std::to_string(dayOrMonth);
    }
    else {
        return "0" + std::to_string(dayOrMonth);
    }
}

tm getDateFromString(std::string dateString) {
    tm tm = {};
    std::istringstream ss(dateString);

    // Parse the date string using std::get_time
    ss >> std::get_time(&tm, "%Y%m%d");

    if (ss.fail()) {
        std::cout << "Date parsing failed!" << std::endl;
        return tm;
    }
    return tm;
}

int AppRunner::runProcessRawMarketdata() {
    // Fetch configuration
    std::string rawFixCapturesLoc = config_.getString("md_raw_fix_file_path");
    std::string startDateStr = config_.getString("start_date", "");
    std::string endDateStr = config_.getString("end_date", "");

    if (startDateStr == "" || endDateStr == "") {
        std::cout << "Required start_date and end_date missing." << std::endl;
    }

    std::cout << "Processing from " << startDateStr << " until " << endDateStr << std::endl;
    tm startDate = getDateFromString(startDateStr);
    tm endDate = getDateFromString(endDateStr);
    tm currentDate = startDate;
    MessageProcessor processor{};

    // Main app loop
    while (true) {
        std::string filePath = rawFixCapturesLoc + kPathSeparator + std::to_string(1900 + currentDate.tm_year) 
            + kPathSeparator + getMonthDayString(currentDate.tm_mon + 1)
            + kPathSeparator + getMonthDayString(currentDate.tm_mday) + ".txt";
        std::ifstream file(filePath);
        if (!file.good()) {
            if (currentDate.tm_mday < 28) {
                std::cout << "Could not find: " << filePath << " exiting." << std::endl;
                break;
            }

            // Try the next month / year
            currentDate.tm_mday = 1;
            currentDate.tm_mon += 1;
            if (currentDate.tm_mon > 11) {
                currentDate.tm_mon += 1;
                currentDate.tm_year += 1;
            }
            filePath = rawFixCapturesLoc + kPathSeparator + std::to_string(1900 + currentDate.tm_year)
                + kPathSeparator + getMonthDayString(currentDate.tm_mon + 1)
                + kPathSeparator + getMonthDayString(currentDate.tm_mday) + ".txt";
        }

        file = std::ifstream(filePath);
        if (!file.good()) {
            std::cout << "Could not find: " << filePath << " exiting." << std::endl;
            break;
        }

        auto end = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(end);
        std::cout << "Processing " << filePath << " started at " << std::ctime(&time) << std::endl;
        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty())
                continue;

            int splitIdx = line.find("|");
            std::string msgIdentifier = line.substr(0, splitIdx);
            std::string msgStr = line.substr(splitIdx + 1);

            FIX::Message msg(msgStr);

            processor.process(msg);
        }
        currentDate.tm_mday += 1;
        if (currentDate.tm_year == endDate.tm_year && currentDate.tm_mon == endDate.tm_mon && currentDate.tm_mday == endDate.tm_mday) {
            std::cout << "Finished processing." << std::endl;
            break;
        }
    }

    // Cleanup
    return 0;
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