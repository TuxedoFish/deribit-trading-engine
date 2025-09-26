#include "../../include/sbe/SBEUtils.h"

#include <stdexcept>
#include <cstring>

using namespace com::liversedge::messages;

int64_t stringToMantissa(const std::string& str, int8_t exponent = -4)
{
    Dec value(str);
    Dec scale = pow(Dec(10), -exponent);
    return static_cast<int64_t>(value * scale);
}

// setVarString is now a template in the header file

void SBEUtils::setQty(Qty& field, const std::string& value)
{
    field.mantissa(stringToMantissa(value, -4));
}

void SBEUtils::setPrice(Price& field, const std::string& value)
{
    field.mantissa(stringToMantissa(value, -8));
}

void SBEUtils::setDate(Date& field, const std::string& date)
{
    int dateInt = std::stoi(date);
    field.year(dateInt / 10000)
         .month((dateInt / 100) % 100)
         .day(dateInt % 100);
}

Currency::Value SBEUtils::currencyFromString(const std::string& currency)
{
    if (currency == "USD")
    {
        return Currency::USD;
    }
    if (currency == "BTC")
    {
        return Currency::BTC;
    }
    if (currency == "ETH")
    {
        return Currency::ETH;
    }
    return Currency::NULL_VALUE;
}

SettlType::Value SBEUtils::settlTypeFromString(const std::string& settlType)
{
    if (settlType == "W1")
    {
        return SettlType::W1;
    }
    if (settlType == "M1")
    {
        return SettlType::M1;
    }
    if (settlType == "M3")
    {
        return SettlType::M3;
    }
    if (settlType == "0")
    {
        return SettlType::REGULAR; // Perp
    }
    return SettlType::NULL_VALUE;
}

SecurityType::Value SBEUtils::securityTypeFromString(const std::string& securityType)
{
    if (securityType == "FUT")
    {
        return SecurityType::FUT;
    }
    if (securityType == "FUTCO")
    {
        return SecurityType::FUTCO;
    }
    return SecurityType::NULL_VALUE;
}

Dec SBEUtils::convertPrice(const Price& price)
{
    int64_t mantissa = price.mantissa();
    int8_t exponent = price.exponent();

    Dec value(mantissa);
    Dec scale = pow(Dec(10), exponent);
    return value * scale;
}

Dec SBEUtils::convertQty(const Qty& qty)
{
    int64_t mantissa = qty.mantissa();
    int8_t exponent = qty.exponent();

    Dec value(mantissa);
    Dec scale = pow(Dec(10), exponent);
    return value * scale;
}

std::string SBEUtils::extractVarString(const VarStringEncoding& varString)
{
    // VarStringEncoding has the length stored first, then the actual string data
    std::uint32_t stringLength = varString.length();

    if (stringLength > 0) {
        // The string data starts after the length field (4 bytes)
        const char* stringStart = varString.buffer() + varString.offset() + varString.varDataEncodingOffset();
        return std::string(stringStart, stringLength);
    } else {
        return "";
    }
}
