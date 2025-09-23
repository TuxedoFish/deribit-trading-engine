#pragma once

#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <sstream>
#include <boost/iostreams/device/mapped_file.hpp>
#include "../util/SimpleConfig.h"
#include "FileMessageProcessor.h"
#include "../marketdata/SBEBinaryWriter.h"
#include "../marketdata/MessageProcessor.h"

const std::string kPathSeparator =
#ifdef _WIN32
"\\";
#else
"/";
#endif

class MarketdataHistoricalRunner
{
public:
    explicit MarketdataHistoricalRunner(const SimpleConfig& config);
    int run();

private:
    SimpleConfig config_;

    std::string getMonthDayString(int dayOrMonth);
    tm getDateFromString(std::string dateString);
    std::string findValidFilePath(const std::string& rawFixCapturesLoc, tm& currentDate);
    size_t countTotalLines(const char* data, const char* end);
    std::string getStringSafe(const char* data, size_t size);
    void logProgress(size_t processedLines, size_t totalLines,
                     const std::chrono::steady_clock::time_point& startTime,
                     std::chrono::steady_clock::time_point& lastProgressTime);
};