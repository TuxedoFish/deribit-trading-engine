#pragma once

#include <unordered_map>
#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/fix44/MarketDataRequest.h"
#include "quickfix/fix44/MarketDataRequestReject.h"
#include "quickfix/fix44/MarketDataSnapshotFullRefresh.h"
#include "quickfix/fix44/MarketDataIncrementalRefresh.h"
#include "quickfix/fix44/SecurityList.h"
#include "quickfix/fix44/Logon.h"
#include "quickfix/fix44/Logout.h"
#include "../sbe/SBEBinaryWriter.h"
#include "../fix/FixCustomTags.h"
#include "../sbe/SBEUtils.h"
#include "../fix/FixUtils.h"
#include "../../generated/com_liversedge_messages/SecurityDefinition.h"
#include "../../generated/com_liversedge_messages/SecurityStatus.h"
#include "../../generated/com_liversedge_messages/SecurityType.h"
#include "../../generated/com_liversedge_messages/ConnectionStatus.h"
#include "../../generated/com_liversedge_messages/MDFullBook.h"
#include "../../generated/com_liversedge_messages/MDUpdate.h"

struct ProcessorSecurityInfo
{
    std::string symbol;
    com::liversedge::messages::SecurityStatusEnum::Value status;
};

// Caps levels to 5000 to ensure they fit in 128kb buffer
constexpr int MAX_LEVELS = 50;

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

    void setShouldOutput(bool shouldOutput);

private:
    SBEBinaryWriter& m_writer;
    std::int32_t securityIdCounter;
    std::vector<ProcessorSecurityInfo> securitiesInfo;
    std::unordered_map<std::string, int> m_symbolToSecurityId;
    com::liversedge::messages::ConnectionStatus m_connectionStatus;
    com::liversedge::messages::SecurityDefinition m_securityDefinition;
    com::liversedge::messages::SecurityStatus m_securityStatus;
    com::liversedge::messages::MDFullBook m_mdFullBook;
    com::liversedge::messages::MDUpdate m_mdUpdate;
    bool m_shouldOutput = true;

    bool UpdateConnectionStatus(com::liversedge::messages::ConnectionStatusEnum::Value value, std::uint64_t timestamp);
    bool UpdateSecurityStatus(int securityId, std::uint64_t timestamp, com::liversedge::messages::SecurityStatusEnum::Value newStatus);
    bool InvalidateState(std::uint64_t timestamp);
    bool RemoveSecurity(int securityId);
    static uint64_t GetSendingTime(FIX44::Message message);
    template<typename T>
    bool ProcessMDEntry(const T& entry, int securityId, uint64_t timestamp);
};
