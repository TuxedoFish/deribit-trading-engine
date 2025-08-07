// NullLogFactory.h
#pragma once

#include "quickfix/Log.h"
#include "quickfix/SessionID.h"
#include <string>

class NullLog : public FIX::Log {
public:
    void clear() override {}
    void backup() override {}
    void onIncoming(const std::string&) override {}
    void onOutgoing(const std::string&) override {}
    void onEvent(const std::string&) override {}
};

class NullLogFactory : public FIX::LogFactory {
public:
    FIX::Log* create() override {
        return new NullLog();
    }

    FIX::Log* create(const FIX::SessionID&) override {
        return new NullLog();
    }

    void destroy(FIX::Log* log) override {
        delete log;
    }
};