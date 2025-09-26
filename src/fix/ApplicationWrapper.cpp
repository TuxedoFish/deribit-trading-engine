//
// Created by markl on 25/09/2025.
//

#include "../../include/fix/ApplicationWrapper.h"

ApplicationWrapper::ApplicationWrapper(SimpleConfig& config)
    : Application(config), m_writer{}, m_processor(m_writer)
{
    m_writer.openNewFile(config.getString("md_file_path") + kPathSeparator + "messages.sbe");
}

void ApplicationWrapper::fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept
{
    Application::fromApp(message, sessionID);
    m_processor.crack(message, sessionID);
}

void ApplicationWrapper::fromAdmin(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept {
    Application::fromApp(message, sessionID);
    m_processor.crack(message, sessionID);
}
