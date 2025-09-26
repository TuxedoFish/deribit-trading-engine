# pragma once

#include "Application.h"
#include "../marketdata/MessageProcessor.h"

class ApplicationWrapper : public Application
{
public:
    explicit ApplicationWrapper(SimpleConfig& config);
    virtual ~ApplicationWrapper() = default;

    // Override FIX::Application interface to pass to processor
    void fromAdmin(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept override;
    void fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept override;

private:
    SBEBinaryWriter m_writer;
    MessageProcessor m_processor;
};
