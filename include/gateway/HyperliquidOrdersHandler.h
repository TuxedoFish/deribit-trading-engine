#pragma once

#include <memory>
#include <iostream>
#include "../sbe/SBEMessageListener.h"
#include "../sbe/SBEBinaryWriter.h"
#include "../sbe/SBEUtils.h"
#include "RefDataHolder.h"
#include "HyperliquidGWApplication.h"
#include "DeribitMessageConverter.h"

class HyperliquidGWApplication;

class HyperliquidOrdersHandler: public SBEMessageListener
{
public:
    explicit HyperliquidOrdersHandler(RefDataHolder& refDataHolder, HyperliquidGWApplication& gwApplication, SBEBinaryWriter& sbeWriter);
    ~HyperliquidOrdersHandler() = default;

    // SBEMessageListener implementation
    void onConnectionStatus(com::liversedge::messages::ConnectionStatus& decoder, std::uint64_t timestamp) override {}
    void onSecurityDefinition(com::liversedge::messages::SecurityDefinition& decoder, std::uint64_t timestamp) override;
    void onSecurityStatus(com::liversedge::messages::SecurityStatus& decoder, std::uint64_t timestamp) override {}
    void onMDUpdate(com::liversedge::messages::MDUpdate& decoder, std::uint64_t timestamp) override {}
    void onMDFullBook(com::liversedge::messages::MDFullBook& decoder, std::uint64_t timestamp) override {}
    void onNewOrder(com::liversedge::messages::NewOrder& decoder, std::uint64_t timestamp) override;
    void onAmendOrder(com::liversedge::messages::AmendOrder& decoder, std::uint64_t timestamp) override;
    void onCancelOrder(com::liversedge::messages::CancelOrder& decoder, std::uint64_t timestamp) override;
    void onOrderCancelReject(com::liversedge::messages::OrderCancelReject& decoder, std::uint64_t timestamp) override {}
    void onExecutionReport(com::liversedge::messages::ExecutionReport& decoder, std::uint64_t timestamp) override {}

    // Access to reference data
    RefDataHolder& getRefDataHolder() { return m_refDataHolder; }
    void setIsReplay (const bool isReplay) { m_isReplay = isReplay; }

private:
    bool m_isReplay = false;
    RefDataHolder& m_refDataHolder;
    HyperliquidGWApplication& m_gwApplication;
    SBEBinaryWriter& m_sbeWriter;

    void sendCancelReject(com::liversedge::messages::CancelOrder& cancelOrder);
    void sendNewOrderReject(com::liversedge::messages::NewOrder& newOrder);

    static hyperliquid::Tif mapTimeInForce(com::liversedge::messages::TimeInForce::Value tif);
};
