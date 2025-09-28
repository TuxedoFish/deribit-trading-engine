#include "../../include/gateway/GWRunner.h"
#include "../../include/gateway/OrdersHandler.h"
#include "../../include/gateway/GWApplication.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

GWRunner::GWRunner(const SimpleConfig& config, GWApplication& gwApplication)
    : m_config(config), m_gwApplication(gwApplication) {
    setupPollers();
}

void GWRunner::run() {
    // Set stdin to non-blocking mode
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    // Replay streams
    std::cout << "Replaying streams..." << std::endl;
    m_ordersHandler->setIsReplay(true);
    while (m_mdPoller->next()) {
    }
    while (m_gwInPoller->next()) {
    }
    m_ordersHandler->setIsReplay(false);
    std::cout << "Finished replay." << std::endl;

    while (true) {
        // Poll the marketdata queue
        while (m_mdPoller->next()) {
        }
        // Poll the inbound orders stream
        while (m_gwInPoller->next()) {
        }

        // Non-blocking check for input
        char c;
        if (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 'q') {
                break;
            }
        }

        // Small sleep to prevent 100% CPU usage
        usleep(1000); // 1ms sleep
    }

    // Restore stdin to blocking mode
    fcntl(STDIN_FILENO, F_SETFL, flags);
}

void GWRunner::setupPollers() {
    auto refDataHolder = std::make_unique<RefDataHolder>();
    m_ordersHandler = std::make_unique<OrdersHandler>(std::move(refDataHolder), m_gwApplication);
    m_mdPoller = createPoller(m_config.getString("md_file_path"), *m_ordersHandler);
    m_gwInPoller = createPoller(m_config.getString("gw_inbound_file_path"), *m_ordersHandler);
}

std::unique_ptr<SBEQueuePoller> GWRunner::createPoller(std::string dataDirectory, SBEMessageListener& listener)
{
    auto poller = std::make_unique<SBEQueuePoller>(dataDirectory, listener);

    boost::filesystem::path path = boost::filesystem::path(dataDirectory) / "messages.sbe";
    std::cout << "Reading from: " << path << std::endl;

    // Only try to read if the file exists, enable live mode for continuous reading
    if (boost::filesystem::exists(path)) {
        poller->readFrom(path, true); // true = live mode
    } else {
        std::cout << "No messages.sbe file found at: " << path << std::endl;
        std::cout << "Will poll for new file creation..." << std::endl;
    }
    return poller;
}
