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
#include <quickfix/FileLog.h>

class FIXRunner
{
public:
    explicit FIXRunner(const SimpleConfig& config);
    ~FIXRunner() = default;

    int run(FIX::Application& application, const std::string& startupMessage);
    int run(FIX::Application& application, const std::string& startupMessage, std::function<void()> mainLoop, bool logFixMessages);
    void runWithInitiator(std::function<void()> mainLoop, FIX::ThreadedSSLSocketInitiator& initiator);

    // Default main loop implementation
    static void waitForUserInput();

private:
    const SimpleConfig& config_;
};