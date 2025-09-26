#ifndef APPLICATION_H
#define APPLICATION_H

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/Values.h"
#include "quickfix/Mutex.h"
#include "quickfix/Session.h"
#include "quickfix/fix44/MarketDataRequest.h"
#include "quickfix/fix44/MarketDataSnapshotFullRefresh.h"
#include "quickfix/fix44/SecurityList.h"
#include <iostream>
#include <chrono>
#include <string>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <algorithm>
#include <vector>
#include "../util/AuthHandler.h"
#include "../marketdata/MarketDataLogger.h"
#include "../util/SimpleConfig.h"
#include "FixUtils.h"

using encoding_t = unsigned char const*;

class MDApplicationBase : public FIX::Application, public FIX::MessageCracker
{
public:
    MDApplicationBase(SimpleConfig& config) : m_config{ config } {}

    // Application interface
    void onCreate(const FIX::SessionID&) override;
    void onLogon(const FIX::SessionID&) override;
    void onLogout(const FIX::SessionID&) override;
    void toAdmin(FIX::Message&, const FIX::SessionID&) override;
    void toApp(FIX::Message&, const FIX::SessionID&) noexcept;
    void fromAdmin(const FIX::Message&, const FIX::SessionID&) noexcept;
    void fromApp(const FIX::Message&, const FIX::SessionID&) noexcept;

    // Deribit marketdata functionality
    void subscribe(std::string[], int);
    void getSymbols();

private:
    FIX::SessionID m_sessionID;
    bool m_loggedOn = false;
    SimpleConfig m_config;

    // Overloaded onMessage
    void onMessage(const FIX44::MarketDataRequest&, const FIX::SessionID&);
    void onMessage(const FIX44::MarketDataRequestReject&, const FIX::SessionID&);
    void onMessage(const FIX44::MarketDataSnapshotFullRefresh&, const FIX::SessionID&);
    void onMessage(const FIX44::MarketDataIncrementalRefresh&, const FIX::SessionID&);
    void onMessage(const FIX44::SecurityList&, const FIX::SessionID&);

};

#endif