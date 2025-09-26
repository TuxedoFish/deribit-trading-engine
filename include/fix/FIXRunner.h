#pragma once

#include <iostream>
#include <string>
#include <functional>
#include "quickfix/Application.h"
#include "quickfix/FileStore.h"
#include "quickfix/SessionSettings.h"
#include "quickfix/ThreadedSSLSocketInitiator.h"
#include "../util/SimpleConfig.h"
#include "../util/NullLogFactory.h"

class FIXRunner
{
public:
    explicit FIXRunner(const SimpleConfig& config);
    ~FIXRunner() = default;

    // Run FIX session with default waitForUserInput main loop
    int run(FIX::Application& application, const std::string& startupMessage);

    // Run FIX session with custom main loop function
    int run(FIX::Application& application, const std::string& startupMessage, std::function<void()> mainLoop);

    // Default main loop implementation
    static void waitForUserInput();

private:
    const SimpleConfig& config_;
};