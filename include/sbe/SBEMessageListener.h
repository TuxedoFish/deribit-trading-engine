#pragma once

#include <cstdint>
#include "../../generated/com_liversedge_messages/ConnectionStatus.h"
#include "../../generated/com_liversedge_messages/SecurityDefinition.h"
#include "../../generated/com_liversedge_messages/SecurityStatus.h"
#include "../../generated/com_liversedge_messages/MDFullBook.h"
#include "../../generated/com_liversedge_messages/MDUpdate.h"

class SBEMessageListener
{
public:
    virtual ~SBEMessageListener() = default;

    virtual void onConnectionStatus(const com::liversedge::messages::ConnectionStatus& decoder, std::uint64_t timestamp) = 0;
    virtual void onSecurityDefinition(const com::liversedge::messages::SecurityDefinition& decoder, std::uint64_t timestamp) = 0;
    virtual void onSecurityStatus(const com::liversedge::messages::SecurityStatus& decoder, std::uint64_t timestamp) = 0;
    virtual void onMDFullBook(const com::liversedge::messages::MDFullBook& decoder, std::uint64_t timestamp) = 0;
    virtual void onMDUpdate(const com::liversedge::messages::MDUpdate& decoder, std::uint64_t timestamp) = 0;
};