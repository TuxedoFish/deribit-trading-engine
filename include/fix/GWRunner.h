#pragma once

#include <iostream>
#include <string>
#include "../util/SimpleConfig.h"

class GWRunner
{
public:
    explicit GWRunner(const SimpleConfig& config);
    ~GWRunner() = default;

    // Instance run method with access to config
    void run();

private:
    const SimpleConfig& config_;
};