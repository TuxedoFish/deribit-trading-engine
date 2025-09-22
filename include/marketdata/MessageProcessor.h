#pragma once

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/fix44/MarketDataRequest.h"
#include "quickfix/fix44/MarketDataSnapshotFullRefresh.h"
#include "quickfix/fix44/SecurityList.h"
#include "SBEBinaryWriter.h"
#include "FixCustomTags.h"
#include "../util/SBEUtils.h"
#include "../../generated/com_liversedge_messages/SecurityDefinition.h"
#include "../../generated/com_liversedge_messages/SecurityType.h"

class MessageProcessor : public FIX::MessageCracker
{
public:
    explicit MessageProcessor(SBEBinaryWriter& writer);
    ~MessageProcessor() override = default;

    void onMessage(const FIX44::MarketDataRequest&, const FIX::SessionID&) override;
    void onMessage(const FIX44::MarketDataRequestReject&, const FIX::SessionID&) override;
    void onMessage(const FIX44::MarketDataSnapshotFullRefresh&, const FIX::SessionID&) override;
    void onMessage(const FIX44::MarketDataIncrementalRefresh&, const FIX::SessionID&) override;
    void onMessage(const FIX44::SecurityList&, const FIX::SessionID&) override;
    void onMessage(const FIX44::Logout&, const FIX::SessionID&) override;

private:
    SBEBinaryWriter& m_writer;
    std::int32_t securityIdCounter;
    com::liversedge::messages::SecurityDefinition m_securityDefinition;

    bool isMultileg(const std::string& symbol);
};
