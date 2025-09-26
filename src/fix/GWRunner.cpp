#include "../../include/fix/GWRunner.h"
#include <boost/filesystem.hpp>

GWRunner::GWRunner(const SimpleConfig& config) : config_(config) {
    setupPollers();
}

void GWRunner::setupPollers() {
    m_refDataHolder = std::make_unique<RefDataHolder>();

    std::string dataDirectory = config_.getString("md_file_path");
    m_mdPoller = std::make_unique<SBEQueuePoller>(dataDirectory, *m_refDataHolder);

    boost::filesystem::path messagesPath = boost::filesystem::path(dataDirectory) / "messages.sbe";

    // Only try to read if the file exists
    if (boost::filesystem::exists(messagesPath)) {
        m_mdPoller->readFrom(messagesPath);
    } else {
        std::cout << "No messages.sbe file found at: " << messagesPath << std::endl;
    }
}

void GWRunner::run() {
    while (true) {
        // Poll the marketdata queue
        while (m_mdPoller->next()) {
        }

        std::string value;
        std::cin >> value;

        if (value == "q") {
            break;
        }

        std::cout << std::endl;
    }
}