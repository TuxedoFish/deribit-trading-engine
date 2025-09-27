#pragma once

#include <memory>
#include "../sbe/SBEMessageListener.h"
#include "RefDataHolder.h"

class OrdersHandler : public SBEMessageListener
{
public:
    explicit OrdersHandler(std::unique_ptr<RefDataHolder> refDataHolder);
    ~OrdersHandler() = default;

    // SBEMessageListener implementation
    void onConnectionStatus(const com::liversedge::messages::ConnectionStatus& decoder, std::uint64_t timestamp) override {}
    void onSecurityDefinition(const com::liversedge::messages::SecurityDefinition& decoder, std::uint64_t timestamp) override;
    void onSecurityStatus(const com::liversedge::messages::SecurityStatus& decoder, std::uint64_t timestamp) override {}
    void onMDUpdate(const com::liversedge::messages::MDUpdate& decoder, std::uint64_t timestamp) override {}
    void onMDFullBook(const com::liversedge::messages::MDFullBook& decoder, std::uint64_t timestamp) override {}
    void onNewOrder(const com::liversedge::messages::NewOrder& decoder, std::uint64_t timestamp) override;
    void onCancelOrder(const com::liversedge::messages::CancelOrder& decoder, std::uint64_t timestamp) override;
    void onOrderCancelReject(const com::liversedge::messages::OrderCancelReject& decoder, std::uint64_t timestamp) override {}
    void onExecutionReport(const com::liversedge::messages::ExecutionReport& decoder, std::uint64_t timestamp) override {}

    // Access to reference data
    RefDataHolder& getRefDataHolder() { return *m_refDataHolder; }
    void setIsReplay (const bool isReplay) { m_isReplay = isReplay; }

private:
    bool m_isReplay = true;
    std::unique_ptr<RefDataHolder> m_refDataHolder;
};