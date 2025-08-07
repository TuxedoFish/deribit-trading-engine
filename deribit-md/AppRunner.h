#pragma once

#include <iostream>
#include <fstream>
#include "Application.h"
#include "SimpleConfig.h"
#include "ApplicationPersister.h"
#include "CmdLineOptions.h"
#include "NullLogFactory.h"
#include "quickfix/FileStore.h"
#include "quickfix/FileLog.h"
#include "quickfix/SocketInitiator.h"
#include "quickfix/SessionSettings.h"
#include "quickfix/ThreadedSSLSocketInitiator.h"

class AppRunner
{
public:
	explicit AppRunner(const SimpleConfig& config);
	int runProcessRawMarketdata();
	int runMarketdataHistoricalStorage();
private:
	SimpleConfig config_;
};