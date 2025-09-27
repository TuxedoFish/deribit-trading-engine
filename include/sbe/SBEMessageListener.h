#pragma once

#include <cstdint>
#include "../../generated/com_liversedge_messages/ConnectionStatus.h"
#include "../../generated/com_liversedge_messages/SecurityDefinition.h"
#include "../../generated/com_liversedge_messages/SecurityStatus.h"
#include "../../generated/com_liversedge_messages/MDFullBook.h"
#include "../../generated/com_liversedge_messages/MDUpdate.h"
#include "../../generated/com_liversedge_messages/NewOrder.h"
#include "../../generated/com_liversedge_messages/CancelOrder.h"
#include "../../generated/com_liversedge_messages/OrderCancelReject.h"
#include "../../generated/com_liversedge_messages/ExecutionReport.h"

class SBEMessageListener
{
public:
    virtual ~SBEMessageListener() = default;

    // Marketdata
    virtual void onConnectionStatus(const com::liversedge::messages::ConnectionStatus& decoder, std::uint64_t timestamp) = 0;
    virtual void onSecurityDefinition(const com::liversedge::messages::SecurityDefinition& decoder, std::uint64_t timestamp) = 0;
    virtual void onSecurityStatus(const com::liversedge::messages::SecurityStatus& decoder, std::uint64_t timestamp) = 0;
    virtual void onMDFullBook(const com::liversedge::messages::MDFullBook& decoder, std::uint64_t timestamp) = 0;
    virtual void onMDUpdate(const com::liversedge::messages::MDUpdate& decoder, std::uint64_t timestamp) = 0;

    // Orders
    virtual void onNewOrder(const com::liversedge::messages::NewOrder& decoder, std::uint64_t timestamp) = 0;
    virtual void onCancelOrder(const com::liversedge::messages::CancelOrder& decoder, std::uint64_t timestamp) = 0;
    virtual void onOrderCancelReject(const com::liversedge::messages::OrderCancelReject& decoder, std::uint64_t timestamp) = 0;
    virtual void onExecutionReport(const com::liversedge::messages::ExecutionReport& decoder, std::uint64_t timestamp) = 0;

};