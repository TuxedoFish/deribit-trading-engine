#pragma once

#include <iostream>
#include <string>
#include <memory>
#include "../util/SimpleConfig.h"
#include "../sbe/SBEQueuePoller.h"
#include "../sbe/RefDataHolder.h"

class GWRunner
{
public:
    explicit GWRunner(const SimpleConfig& config);
    ~GWRunner() = default;

    // Instance run method with access to config
    void run();

private:
    const SimpleConfig& config_;
    std::unique_ptr<RefDataHolder> m_refDataHolder;
    std::unique_ptr<SBEQueuePoller> m_mdPoller;

    void setupPollers();
};