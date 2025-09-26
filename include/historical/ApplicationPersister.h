// ApplicationPersister.h - Logging extension
#pragma once

#include "../fix/MDApplicationBase.h"
#include "../marketdata/MarketDataLogger.h"
#include "../util/SimpleConfig.h"
#include <memory>
#include <string>

class ApplicationPersister : public MDApplicationBase {
private:
    std::unique_ptr<MarketDataLogger> m_logger;

public:
    // Constructor with optional log directory
    explicit ApplicationPersister(SimpleConfig& config);
    virtual ~ApplicationPersister() = default;

    // Override FIX::Application interface to add logging
    void toApp(FIX::Message& message, const FIX::SessionID& sessionID) noexcept override;
    void fromAdmin(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept override;
    void fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept override;

    // Configuration methods
    void setLogDirectory(const std::string& directory);
    std::string getLogDirectory() const;
    bool isLoggingActive() const;
};