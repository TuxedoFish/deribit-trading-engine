#include "../../include/historical/ApplicationPersister.h"

ApplicationPersister::ApplicationPersister(SimpleConfig& config)
    : Application(config), m_logger(std::make_unique<MarketDataLogger>(config.getString("md_raw_fix_file_path"))) {
}

void ApplicationPersister::toApp(FIX::Message& message, const FIX::SessionID& sessionID) noexcept {
    if (m_logger) {
        m_logger->writeToLog("OUT", message);
    }
    Application::toApp(message, sessionID);
}

void ApplicationPersister::fromAdmin(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept {
    if (m_logger) {
        m_logger->writeToLog("IN_ADMIN", message);
    }
    Application::fromAdmin(message, sessionID);
}

void ApplicationPersister::fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept {
    if (m_logger) {
        m_logger->writeToLog("IN_APP", message);
    }
    Application::fromApp(message, sessionID);
}

void ApplicationPersister::setLogDirectory(const std::string& directory) {
    if (m_logger) {
        m_logger->setLogDirectory(directory);
    }
}

std::string ApplicationPersister::getLogDirectory() const {
    if (m_logger) {
        return m_logger->getLogDirectory();
    }
    return "";
}

bool ApplicationPersister::isLoggingActive() const {
    return m_logger && m_logger->isLogFileOpen();
}
