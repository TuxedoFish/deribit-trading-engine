#pragma once

#include <iostream>
#include <string>
#include <memory>
#include "../util/SimpleConfig.h"
#include "../sbe/SBEQueuePoller.h"
#include "RefDataHolder.h"

class GWRunner
{
public:
    explicit GWRunner(const SimpleConfig& config);
    ~GWRunner() = default;

    // Instance run method with access to config
    void run();

private:
    const SimpleConfig& m_config;
    std::unique_ptr<RefDataHolder> m_refDataHolder;
    std::unique_ptr<SBEQueuePoller> m_mdPoller;
    std::unique_ptr<SBEQueuePoller> m_gwInPoller;

    void setupPollers();
    std::unique_ptr<SBEQueuePoller> createPoller(std::string directory, SBEMessageListener& listener);
};