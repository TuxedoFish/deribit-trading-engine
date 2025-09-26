#include "../../include/fix/GWRunner.h"
#include <boost/filesystem.hpp>

GWRunner::GWRunner(const SimpleConfig& config) : m_config(config) {
    setupPollers();
}

void GWRunner::run() {
    while (true) {
        // Poll the marketdata queue
        while (m_mdPoller->next()) {
        }
        // Poll the inbound orders stream
        while (m_gwInPoller->next()) {
        }

        std::string value;
        std::cin >> value;

        if (value == "q") {
            break;
        }

        std::cout << std::endl;
    }
}

void GWRunner::setupPollers() {
    m_refDataHolder = std::make_unique<RefDataHolder>();
    m_mdPoller = createPoller(m_config.getString("md_file_path"), *m_refDataHolder);
    m_gwInPoller = createPoller(m_config.getString("gw_inbound_file_path"), *m_refDataHolder);
}

std::unique_ptr<SBEQueuePoller> GWRunner::createPoller(std::string dataDirectory, SBEMessageListener& listener)
{
    auto poller = std::make_unique<SBEQueuePoller>(dataDirectory, listener);

    boost::filesystem::path path = boost::filesystem::path(dataDirectory) / "messages.sbe";
    std::cout << "Reading from: " << path << std::endl;

    // Only try to read if the file exists
    if (boost::filesystem::exists(path)) {
        poller->readFrom(path);
    } else {
        std::cout << "No messages.sbe file found at: " << path << std::endl;
    }
    return poller;
}
