#include "ApplicationPersister.h"

ApplicationPersister::ApplicationPersister(const std::string& logDirectory)
    : Application(), m_logger(std::make_unique<MarketDataLogger>(logDirectory)) {
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
