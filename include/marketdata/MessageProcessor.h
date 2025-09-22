#pragma once

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/fix44/MarketDataRequest.h"
#include "quickfix/fix44/MarketDataRequestReject.h"
#include "quickfix/fix44/MarketDataSnapshotFullRefresh.h"
#include "quickfix/fix44/MarketDataIncrementalRefresh.h"
#include "quickfix/fix44/SecurityList.h"
#include "quickfix/fix44/Logon.h"
#include "quickfix/fix44/Logout.h"
#include "SBEBinaryWriter.h"
#include "FixCustomTags.h"
#include "../util/SBEUtils.h"
#include "FixUtils.h"
#include "../../generated/com_liversedge_messages/SecurityDefinition.h"
#include "../../generated/com_liversedge_messages/SecurityStatus.h"
#include "../../generated/com_liversedge_messages/SecurityType.h"
#include "../../generated/com_liversedge_messages/ConnectionStatus.h"
#include "../../generated/com_liversedge_messages/MDFullBook.h"

struct SecurityInfo
{
    std::string symbol;
    com::liversedge::messages::SecurityStatusEnum::Value status;
};

// Caps levels to 5000 to ensure they fit in 128kb buffer
constexpr int MAX_LEVELS = 5000;

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
    void onMessage(const FIX44::Logon&, const FIX::SessionID&) override;

private:
    SBEBinaryWriter& m_writer;
    std::int32_t securityIdCounter;
    std::vector<SecurityInfo> securitiesInfo;
    com::liversedge::messages::ConnectionStatus m_connectionStatus;
    com::liversedge::messages::SecurityDefinition m_securityDefinition;
    com::liversedge::messages::SecurityStatus m_securityStatus;
    com::liversedge::messages::MDFullBook m_mdFullBook;

    bool UpdateConnectionStatus(com::liversedge::messages::ConnectionStatusEnum::Value value, std::uint64_t timestamp);
    bool UpdateSecurityStatus(int securityId, std::uint64_t timestamp, com::liversedge::messages::SecurityStatusEnum::Value newStatus);
    bool InvalidateState(std::uint64_t timestamp);
    static uint64_t GetSendingTime(FIX44::Message message);
};
