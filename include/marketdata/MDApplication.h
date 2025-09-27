# pragma once

#include "MDApplicationBase.h"
#include "MessageProcessor.h"

class MDApplication : public MDApplicationBase
{
public:
    explicit MDApplication(SimpleConfig& config);
    virtual ~MDApplication() = default;

    // Override FIX::Application interface to pass to processor
    void fromAdmin(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept override;
    void fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) noexcept override;

private:
    SBEBinaryWriter m_writer;
    MessageProcessor m_processor;
};
