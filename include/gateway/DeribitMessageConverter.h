#pragma once

#include "quickfix/fix44/OrderCancelReject.h"
#include "quickfix/fix44/ExecutionReport.h"
#include "../../generated/com_liversedge_messages/OrderCancelReject.h"
#include "../../generated/com_liversedge_messages/ExecutionReport.h"
#include "RefDataHolder.h"

class DeribitMessageConverter
{
public:
    static void convertOrderCancelReject(
        const FIX44::OrderCancelReject& fixMessage,
        com::liversedge::messages::OrderCancelReject& sbeMessage,
        RefDataHolder& refDataHolder
    );

    static void convertExecutionReport(
        const FIX44::ExecutionReport& fixMessage,
        com::liversedge::messages::ExecutionReport& sbeMessage,
        RefDataHolder& refDataHolder
    );

private:
    DeribitMessageConverter() = default;
    static std::uint64_t extractSendingTimeFromFix(const FIX::Message& message);
};