#pragma once

#include <iostream>
#include <string>
#include <memory>
#include "../util/SimpleConfig.h"
#include "../sbe/SBEQueuePoller.h"
#include "OrdersHandler.h"

// Forward declaration
class GWApplication;
class RefDataHolder;

class GWRunner
{
public:
    explicit GWRunner(const SimpleConfig& config, GWApplication& gwApplication, RefDataHolder& refDataHolder);
    ~GWRunner() = default;

    // Instance run method with access to config
    void run();

private:
    const SimpleConfig& m_config;
    GWApplication& m_gwApplication;
    RefDataHolder& m_refDataHolder;
    std::unique_ptr<OrdersHandler> m_ordersHandler;
    std::unique_ptr<SBEQueuePoller> m_mdPoller;
    std::unique_ptr<SBEQueuePoller> m_gwInPoller;

    void setupPollers();
    std::unique_ptr<SBEQueuePoller> createPoller(std::string directory, SBEMessageListener& listener);
};