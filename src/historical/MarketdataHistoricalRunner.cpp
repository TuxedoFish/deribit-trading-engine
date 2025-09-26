#include "../../include/historical/MarketdataHistoricalRunner.h"

MarketdataHistoricalRunner::MarketdataHistoricalRunner(const SimpleConfig& config) : config_{ config } {
}

std::string MarketdataHistoricalRunner::getMonthDayString(int dayOrMonth) {
    if (dayOrMonth > 9) {
        return std::to_string(dayOrMonth);
    }
    else {
        return "0" + std::to_string(dayOrMonth);
    }
}

std::string MarketdataHistoricalRunner::findValidFilePath(const std::string& rawFixCapturesLoc, tm& currentDate) {
    std::string datePath = std::to_string(1900 + currentDate.tm_year)
        + kPathSeparator + getMonthDayString(currentDate.tm_mon + 1)
        + kPathSeparator + getMonthDayString(currentDate.tm_mday);
    std::string filePath = rawFixCapturesLoc + kPathSeparator + datePath + ".txt";

    if (!boost::filesystem::exists(filePath)) {
        if (currentDate.tm_mday < 28) {
            return ""; // Signal file not found
        }

        // Try the next month / year
        currentDate.tm_mday = 1;
        currentDate.tm_mon += 1;
        if (currentDate.tm_mon > 11) {
            currentDate.tm_mon = 0; // Reset to January
            currentDate.tm_year += 1;
        }
        datePath = std::to_string(1900 + currentDate.tm_year)
            + kPathSeparator + getMonthDayString(currentDate.tm_mon + 1)
            + kPathSeparator + getMonthDayString(currentDate.tm_mday);
        filePath = rawFixCapturesLoc + kPathSeparator + datePath + ".txt";
    }

    return filePath;
}

size_t MarketdataHistoricalRunner::countTotalLines(const char* data, const char* end) {
    size_t totalLines = 0;
    const char* countPtr = data;
    while (countPtr < end) {
        if (*countPtr == '\n') totalLines++;
        countPtr++;
    }
    return totalLines;
}

std::string MarketdataHistoricalRunner::getStringSafe(const char* data, size_t size) {
    std::string msgStr(data, size);

    bool hasNullBytes = false;
    std::string cleanedMsgStr;

    for (char c : msgStr) {
        if (c == '\0') {
            hasNullBytes = true;
        } else {
            cleanedMsgStr += c;
        }
    }

    if (hasNullBytes) {
        std::cerr << "WARNING: Null bytes detected in message. Original length: "
                  << msgStr.size() << ", cleaned length: " << cleanedMsgStr.size()
                  << " - DISCARDING MESSAGE due to corruption" << std::endl;
        std::cerr << "Corrupted message preview: ";
        for (char c : cleanedMsgStr) {
            if (c == '\x01') std::cerr << "[SOH]";
            else if (std::isprint(c)) std::cerr << c;
            else std::cerr << "[" << static_cast<int>(c) << "]";
        }
        std::cerr << std::endl;
        return "";
    }

    return msgStr;
}

void MarketdataHistoricalRunner::logProgress(size_t processedLines, size_t totalLines,
                                             const std::chrono::steady_clock::time_point& startTime,
                                             std::chrono::steady_clock::time_point& lastProgressTime) {
    auto currentTime = std::chrono::steady_clock::now();
    auto timeSinceLastProgress = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastProgressTime).count();

    if (timeSinceLastProgress >= 5) {
        double progressPercent = (static_cast<double>(processedLines) / totalLines) * 100.0;
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();

        if (progressPercent > 0) {
            double estimatedTotalSeconds = (elapsedTime * 100.0) / progressPercent;
            double remainingSeconds = estimatedTotalSeconds - elapsedTime;

            int remainingHours = static_cast<int>(remainingSeconds) / 3600;
            int remainingMinutes = (static_cast<int>(remainingSeconds) % 3600) / 60;
            int remainingSecs = static_cast<int>(remainingSeconds) % 60;

            std::cout << "Progress: " << std::fixed << std::setprecision(2) << progressPercent
                      << "% (" << processedLines << "/" << totalLines << " lines) - "
                      << "Estimated time remaining: " << remainingHours << "h "
                      << remainingMinutes << "m " << remainingSecs << "s" << std::endl;
        }

        lastProgressTime = currentTime;
    }
}

int MarketdataHistoricalRunner::run() {
    // Fetch configuration
    const std::string rawFixCapturesLoc = config_.getString("md_raw_fix_file_path");
    const std::string processedCapturesLoc = config_.getString("md_processed_file_path");
    const std::string dataDictionaryLoc = config_.getString("data_dictionary_file_path");
    const std::string startDateStr = config_.getString("start_date", "");
    const std::string endDateStr = config_.getString("end_date", "");

    if (startDateStr == "" || endDateStr == "") {
        std::cout << "Required start_date and end_date missing." << std::endl;
    }

    std::cout << "Processing from " << startDateStr << " until " << endDateStr << std::endl;
    tm startDate = DateUtils::getDateFromString(startDateStr);
    tm endDate = DateUtils::getDateFromString(endDateStr);
    tm currentDate = startDate;
    SBEBinaryWriter writer{};
    MessageProcessor processor{ writer };
    FileMessageProcessor historicalProcessor{ dataDictionaryLoc, processor, writer };

    // Check for previous day file and prime state if needed
    tm previousDate = startDate;
    previousDate.tm_mday -= 1;

    // TODO: Handle month/year rollover properly

    std::string previousFilePath = findValidFilePath(rawFixCapturesLoc, previousDate);

    if (!previousFilePath.empty() && boost::filesystem::exists(previousFilePath)) {
        std::cout << "Found previous day file: " << previousFilePath << ", priming state..." << std::endl;

        // Disable output while priming state
        processor.setShouldOutput(false);

        // Read file backwards to find last logon
        boost::iostreams::mapped_file_source previousFile(previousFilePath);
        const char* data = previousFile.data();
        const char* end = data + previousFile.size();

        // Find lines in reverse order
        const char* lineEnd = end;
        bool foundLogon = false;
        size_t linesToReplay = 0;
        const char* replayFrom;

        // Go backwards through the file to collect all lines
        for (const char* pos = end - 1; pos >= data; --pos) {
            if (*pos == '\n' || pos == data) {
                const char* lineStart = (pos == data) ? pos : pos + 1;
                if (lineEnd > lineStart) {
                    const char* pipePos = std::find(lineStart, lineEnd, '|');
                    if (pipePos != lineEnd) {
                        // Extract message part after pipe
                        std::string msgStr = getStringSafe(pipePos + 1, lineEnd - pipePos - 1);
                        linesToReplay++;
                        if (!msgStr.empty() & FileMessageProcessor::isLogon(msgStr)) {
                            replayFrom = lineStart;
                            foundLogon = true;
                            break;
                        }
                    }
                }
                lineEnd = pos;
            }
        }

        if (foundLogon) {
            readFrom(replayFrom, end, historicalProcessor, linesToReplay);
        } else {
            std::cerr << "No logon message found in previous day file" << std::endl;
            return 0;
        }

        previousFile.close();
    } else {
        std::cerr << "No previous day file found (" << previousFilePath << "), exiting early" << std::endl;
        return 0;
    }

    // Main app loop
    while (true) {
        // Re-enable output for normal processing
        processor.setShouldOutput(true);

        std::string filePath = findValidFilePath(rawFixCapturesLoc, currentDate);

        if (filePath.empty() || !boost::filesystem::exists(filePath)) {
            std::cout << "Could not find: " << filePath << " exiting." << std::endl;
            break;
        }

        // Construct datePath for output file
        std::string datePath = std::to_string(1900 + currentDate.tm_year)
            + kPathSeparator + getMonthDayString(currentDate.tm_mon + 1)
            + kPathSeparator + getMonthDayString(currentDate.tm_mday);

        auto now = std::chrono::system_clock::now();
        std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
        std::cout << "Processing " << filePath << " started at " << std::ctime(&nowTime);
        boost::iostreams::mapped_file_source file(filePath);
        historicalProcessor.nextFile(processedCapturesLoc + kPathSeparator + datePath);

        const char* data = file.data();
        const char* end = data + file.size();

        // Count total lines for progress tracking
        size_t totalLines = countTotalLines(data, end);
        std::cout << "Total lines to process: " << totalLines << std::endl;

        const char* lineStart = data;

        auto startTime = std::chrono::steady_clock::now();
        readFrom(lineStart, end, historicalProcessor, totalLines);
        auto endTime = std::chrono::steady_clock::now();

        // Log completion for this file
        auto totalTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
        int hours = totalTime / 3600;
        int minutes = (totalTime % 3600) / 60;
        int seconds = totalTime % 60;

        std::cout << "Completed processing " << filePath << " in " << hours << "h "
                  << minutes << "m " << seconds << "s" << std::endl;

        currentDate.tm_mday += 1;
        if (currentDate.tm_year == endDate.tm_year && currentDate.tm_mon == endDate.tm_mon && currentDate.tm_mday == endDate.tm_mday) {
            std::cout << "Finished processing." << std::endl;
            break;
        }
    }

    // Cleanup
    return 0;
}

void MarketdataHistoricalRunner::readFrom(const char* lineStart, const char* end, FileMessageProcessor& historicalProcessor, int totalLines)
{
    // Progress tracking variables
    auto startTime = std::chrono::steady_clock::now();
    auto lastProgressTime = startTime;
    size_t processedLines = 0;

    while (lineStart < end) {
        const char* lineEnd = std::find(lineStart, end, '\n');

        if (lineEnd == lineStart) {
            lineStart++;
            continue;
        }

        processedLines++;

        const char* pipePos = std::find(lineStart, lineEnd, '|');
        if (pipePos != lineEnd) {
            // Find the actual end of the message data (before \r\n)
            const char* msgEnd = lineEnd;

            // Back up past any \r characters (Windows)
            while (msgEnd > pipePos + 1 && *(msgEnd - 1) == '\r') {
                msgEnd--;
            }

            // Create string_view that preserves binary data including SOH

            // Convert to string while checking for null bytes and filtering them
            std::string msgStr = getStringSafe(pipePos + 1, msgEnd - pipePos - 1);

            // Skip processing if message was corrupted (contains null bytes)
            if (msgStr.empty()) {
                lineStart = lineEnd + 1;
                continue;
            }

            try {
                historicalProcessor.process(msgStr);
            }
            catch (const std::exception& e) {
                std::cerr << "FIX parsing error: " << e.what() << std::endl;
                std::cerr << "Message was: ";
                for (char c : msgStr) {
                    if (c == '\x01') std::cout << "[SOH]";
                    else if (std::isprint(c)) std::cout << c;
                    else std::cout << "[" << static_cast<int>(c) << "]";
                }
                std::cout << std::endl;
            }
        }

        logProgress(processedLines, totalLines, startTime, lastProgressTime);

        lineStart = lineEnd + 1;
    }
}