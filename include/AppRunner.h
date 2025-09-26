#pragma once

#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <boost/iostreams/device/mapped_file.hpp>
#include <string_view>
#include <filesystem>
#include "fix/Application.h"
#include "fix/ApplicationWrapper.h"
#include "util/SimpleConfig.h"
#include "historical/ApplicationPersister.h"
#include "util/CmdLineOptions.h"
#include "util/NullLogFactory.h"
#include "marketdata/MessageProcessor.h"
#include "quickfix/FileStore.h"
#include "quickfix/FileLog.h"
#include "quickfix/SocketInitiator.h"
#include "quickfix/SessionSettings.h"
#include "quickfix/ThreadedSSLSocketInitiator.h"
#include "historical/FileMessageProcessor.h"
#include "historical/MarketdataHistoricalRunner.h"
#include "fix/FIXRunner.h"

class AppRunner
{
public:
	explicit AppRunner(const SimpleConfig& config);
	int runMarketdata();
	int runProcessRawMarketdata();
	int runMarketdataHistoricalStorage();
private:
	SimpleConfig config_;
};