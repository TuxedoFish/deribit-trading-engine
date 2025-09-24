#include "../include/util/SBEUtils.h"

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
