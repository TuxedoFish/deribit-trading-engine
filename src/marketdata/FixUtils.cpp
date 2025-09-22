//
// Created by markl on 22/09/2025.
//

#include "../../include/marketdata/FixUtils.h"
#include <chrono>

std::uint64_t FixUtils::convertFIXTimeToNanos(const FIX::UtcTimeStamp& fixTime)
{
    auto timePoint = std::chrono::system_clock::from_time_t(fixTime.getTimeT());

    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
        timePoint.time_since_epoch()
    ).count();

    nanoseconds += fixTime.getMillisecond() * 1000000;

    return static_cast<std::uint64_t>(nanoseconds);
}
