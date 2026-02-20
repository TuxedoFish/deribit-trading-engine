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
#include "../sbe/SBEBinaryWriter.h"
#include "../marketdata/DeribitMessageProcessor.h"
#include "../util/DateUtils.h"

class MarketdataHistoricalRunner
{
public:
    explicit MarketdataHistoricalRunner(SimpleConfig& config);
    int run();

private:
    SimpleConfig& config_;

    std::string getMonthDayString(int dayOrMonth);
    std::string findValidFilePath(const std::string& rawFixCapturesLoc, tm& currentDate);
    size_t countTotalLines(const char* data, const char* end);
    std::string getStringSafe(const char* data, size_t size);
    void readFrom(const char* lineStart, const char* end, FileMessageProcessor& historicalProcessor, int totalLines);
    void logProgress(size_t processedLines, size_t totalLines,
                     const std::chrono::steady_clock::time_point& startTime,
                     std::chrono::steady_clock::time_point& lastProgressTime);
};