#pragma once

#include <iostream>
#include <string>
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

    // Run FIX session with the provided application and startup message
    int run(FIX::Application& application, const std::string& startupMessage);

private:
    const SimpleConfig& config_;

    void waitForUserInput();
};