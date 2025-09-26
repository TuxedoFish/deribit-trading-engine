#pragma once

#include <unordered_map>
#include <memory>
#include "SBEMessageListener.h"
#include "SecurityInfo.h"
#include "../../generated/com_liversedge_messages/ActionEnum.h"

class RefDataHolder : public SBEMessageListener
{
public:
    RefDataHolder() = default;

    // SBEMessageListener implementation
    void onConnectionStatus(const com::liversedge::messages::ConnectionStatus& decoder, std::uint64_t timestamp) override {}
    void onSecurityDefinition(const com::liversedge::messages::SecurityDefinition& decoder, std::uint64_t timestamp) override;
    void onSecurityStatus(const com::liversedge::messages::SecurityStatus& decoder, std::uint64_t timestamp) override {}
    void onMDUpdate(const com::liversedge::messages::MDUpdate& decoder, std::uint64_t timestamp) override {}
    void onMDFullBook(const com::liversedge::messages::MDFullBook& decoder, std::uint64_t timestamp) override {}

    // Security lookup
    const SecurityInfo* getSecurityInfo(std::int32_t securityId) const;

private:
    std::unordered_map<std::int32_t, std::unique_ptr<SecurityInfo>> m_securities;
};