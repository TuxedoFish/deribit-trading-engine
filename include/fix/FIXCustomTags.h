# pragma once

#include "quickfix/Field.h"

namespace FIX
{
    namespace FIELD
    {
        const int InstrumentPricePrecision = 2576;
        const int DeribitSkipBlockTrades = 9011;
        const int DeribitShowBlockTradeId = 9012;
    }

    DEFINE_BOOLEAN(DeribitSkipBlockTrades);
    DEFINE_BOOLEAN(DeribitShowBlockTradeId);

    const std::string ExecInst_PARTICIPATE_DONT_INITIATE_NO_CROSS = "6A";
}