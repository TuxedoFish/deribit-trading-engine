#pragma once

#include "../../generated/com_liversedge_messages/VarStringEncoding.h"
#include "../../generated/com_liversedge_messages/SecurityDefinition.h"
#include "../../generated/com_liversedge_messages/Currency.h"
#include "../../generated/com_liversedge_messages/SecurityType.h"
#include "../../generated/com_liversedge_messages/SettlType.h"
// #include "../../generated/com_liversedge_messages/Qty.h"
#include "../../generated/com_liversedge_messages/Price.h"
#include "../../generated/com_liversedge_messages/Date.h"
#include <boost/multiprecision/cpp_dec_float.hpp>


using Dec = boost::multiprecision::cpp_dec_float_50;

/**
* Utility class for working with SBE (Simple Binary Encoding) generated types
 */
class SBEUtils {
public:
    /**
     * Sets a VarStringEncoding field with bounds checking
     * @param field The VarStringEncoding field to set
     * @param value The string value to set
     * @throws std::runtime_error if string is too long
     */
    static void setVarString(com::liversedge::messages::VarStringEncoding& field, const std::string& value);

    /**
     * Sets a Qty field from a double value
     * @param field The Qty field to set
     * @param value The string value to convert and set
     */
    static void setQty(com::liversedge::messages::Qty& field, const std::string& value);

    /**
     * Sets a Price field from a double value
     * @param field The Price field to set
     * @param value The string value to convert and set
     */
    static void setPrice(com::liversedge::messages::Price& field, const std::string& value);

    /**
     * Sets a Date field from year, month, day
     * @param field The Date field to set
     * @param value The string value to convert and set
     */
    static void setDate(com::liversedge::messages::Date& field, const std::string& value);

    /**
     * Get currency from field
     * @param currency The Currency in string form
     */
    static com::liversedge::messages::Currency::Value currencyFromString(const std::string& currency);

    /**
     * Get settle type from field
     * @param settlType The SettlType in string form
     */
    static com::liversedge::messages::SettlType::Value settlTypeFromString(const std::string& settlType);

    /**
     * Get security type from field
     * @param securityType The SecurityType in string form
     */
    static com::liversedge::messages::SecurityType::Value securityTypeFromString(const std::string& securityType);

private:
    // Private constructor - this is a utility class with only static methods
    SBEUtils() = default;
};