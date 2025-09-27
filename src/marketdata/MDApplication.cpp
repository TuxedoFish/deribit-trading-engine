//
// Created by markl on 25/09/2025.
//

#include "../../include/marketdata/MDApplication.h"

MDApplication::MDApplication(SimpleConfig& config)
    : MDApplicationBase(config), m_writer{}, m_processor(m_writer)
{
    m_writer.openNewFile(config.getString("md_file_path") + kPathSeparator + "messages.sbe");
}

void MDApplication::fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept
{
    MDApplicationBase::fromApp(message, sessionID);
    m_processor.crack(message, sessionID);
}

void MDApplication::fromAdmin(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept {
    MDApplicationBase::fromApp(message, sessionID);
    m_processor.crack(message, sessionID);
}
