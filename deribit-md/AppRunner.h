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
#include "Application.h"
#include "SimpleConfig.h"
#include "ApplicationPersister.h"
#include "CmdLineOptions.h"
#include "NullLogFactory.h"
#include "MessageProcessor.h"
#include "quickfix/FileStore.h"
#include "quickfix/FileLog.h"
#include "quickfix/SocketInitiator.h"
#include "quickfix/SessionSettings.h"
#include "quickfix/ThreadedSSLSocketInitiator.h"

const std::string kPathSeparator =
#ifdef _WIN32
"\\";
#else
"/";
#endif

class AppRunner
{
public:
	explicit AppRunner(const SimpleConfig& config);
	int runProcessRawMarketdata();
	int runMarketdataHistoricalStorage();
private:
	SimpleConfig config_;
};