# pragma once

#include <cstdint>
#include <quickfix/FieldTypes.h>

class FixUtils
{
    public:
    static std::uint64_t convertFIXTimeToNanos(const FIX::UtcTimeStamp& fixTime);
};
