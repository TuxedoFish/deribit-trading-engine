#include "../../include/gateway/SecurityInfo.h"
#include "../../include/sbe/SBEUtils.h"
#include <sstream>

SecurityInfo::SecurityInfo(const com::liversedge::messages::SecurityDefinition& secDef)
{
    m_id = secDef.id();

    // Extract symbol string from variable-length field
    auto symbolField = const_cast<com::liversedge::messages::SecurityDefinition&>(secDef).symbol();
    m_symbol = SBEUtils::extractVarString(symbolField, secDef.sbeBlockLength());

    m_currency = secDef.currency();
    m_commCurrency = secDef.commCurrency();
    m_settlCurrency = secDef.settlCurrency();
    m_settlType = secDef.settlType();

    // Convert SBE Date to std::tm
    auto maturityDate = const_cast<com::liversedge::messages::SecurityDefinition&>(secDef).maturityDate();
    m_maturityDate.tm_year = maturityDate.year() - 1900;  // tm_year is years since 1900
    m_maturityDate.tm_mon = maturityDate.month() - 1;     // tm_mon is 0-11
    m_maturityDate.tm_mday = maturityDate.day();

    // Convert Price and Qty fields using SBEUtils
    auto minPriceIncrementField = const_cast<com::liversedge::messages::SecurityDefinition&>(secDef).minPriceIncrement();
    m_minPriceIncrement = SBEUtils::convertPrice(minPriceIncrementField);

    m_instrumentPricePrecision = secDef.instrumentPricePrecision();

    auto minSizeIncrementField = const_cast<com::liversedge::messages::SecurityDefinition&>(secDef).minSizeIncrement();
    m_minSizeIncrement = SBEUtils::convertQty(minSizeIncrementField);

    auto contractMultiplierField = const_cast<com::liversedge::messages::SecurityDefinition&>(secDef).contractMultiplier();
    m_contractMultiplier = SBEUtils::convertPrice(contractMultiplierField);

    m_securityType = secDef.securityType();
}

std::string SecurityInfo::toString() const
{
    std::ostringstream oss;
    oss << "SecurityInfo{";
    oss << "id=" << m_id;
    oss << ", symbol='" << m_symbol << "'";
    oss << ", currency=" << static_cast<int>(m_currency);
    oss << ", commCurrency=" << static_cast<int>(m_commCurrency);
    oss << ", settlCurrency=" << static_cast<int>(m_settlCurrency);
    oss << ", settlType=" << static_cast<int>(m_settlType);
    oss << ", maturityDate=" << (m_maturityDate.tm_year + 1900) << "-"
        << (m_maturityDate.tm_mon + 1) << "-" << m_maturityDate.tm_mday;
    oss << ", minPriceIncrement=" << m_minPriceIncrement;
    oss << ", instrumentPricePrecision=" << static_cast<int>(m_instrumentPricePrecision);
    oss << ", minSizeIncrement=" << m_minSizeIncrement;
    oss << ", contractMultiplier=" << m_contractMultiplier;
    oss << ", securityType=" << static_cast<int>(m_securityType);
    oss << "}";
    return oss.str();
}