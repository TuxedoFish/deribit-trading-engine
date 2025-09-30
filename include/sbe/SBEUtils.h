#pragma once

#include "../../generated/com_liversedge_messages/VarStringEncoding.h"
#include "../../generated/com_liversedge_messages/SecurityDefinition.h"
#include "../../generated/com_liversedge_messages/Currency.h"
#include "../../generated/com_liversedge_messages/SecurityType.h"
#include "../../generated/com_liversedge_messages/SettlType.h"
#include "../../generated/com_liversedge_messages/Qty.h"
#include "../../generated/com_liversedge_messages/Price.h"
#include "../../generated/com_liversedge_messages/Date.h"
#include "../../generated/com_liversedge_messages/Side.h"
#include "../../generated/com_liversedge_messages/OrderType.h"
#include "../../generated/com_liversedge_messages/TimeInForce.h"
#include "../../generated/com_liversedge_messages/OrdStatus.h"
#include "../util/DecimalTypes.h"
#include "quickfix/Fields.h"

#define HEADER_LENGTH 8

/**
* Utility class for working with SBE (Simple Binary Encoding) generated types
 */
class SBEUtils {
public:
    /**
     * Sets a VarStringEncoding field with bounds checking and advances parent message position
     * @param parentMessage The parent SBE message (to advance sbePosition)
     * @param field The VarStringEncoding field to set
     * @param value The string value to set
     * @throws std::runtime_error if string is too long
     */
    template<typename T>
    static void setVarString(T& parentMessage, com::liversedge::messages::VarStringEncoding& field, const std::string& value)
    {
        if (value.length() > com::liversedge::messages::VarStringEncoding::lengthMaxValue())
        {
            throw std::runtime_error("String too long for VarStringEncoding: " +
                std::to_string(value.length()) + " > " +
                std::to_string(com::liversedge::messages::VarStringEncoding::lengthMaxValue()));
        }

        // Update the field in place
        field.length(static_cast<std::uint32_t>(value.length()));
        if (value.length() > 0)
        {
            // Append variable data to the end
            std::memcpy(field.buffer() + parentMessage.sbePosition(), value.c_str(), value.length());
        }

        // Advance parent message position by the string length (length field is already included in encodedLength)
        parentMessage.sbePosition(parentMessage.sbePosition() + value.length());
    }

    // Setting SBE
    static void setQty(com::liversedge::messages::Qty& field, const std::string& value);
    static void setPrice(com::liversedge::messages::Price& field, const std::string& value);
    static void setDate(com::liversedge::messages::Date& field, const std::string& value);
    // Buffer access helpers
    static std::int64_t getInt64(const char* buffer, std::size_t offset);
    // SBE -> Internal
    static Dec convertPrice(const com::liversedge::messages::Price& price);
    static Dec convertQty(const com::liversedge::messages::Qty& qty);
    static std::string extractVarString(const com::liversedge::messages::VarStringEncoding& varString, const int encodedLength, const int variableOffset = 0);
    // FIX -> SBE
    static com::liversedge::messages::Currency::Value currencyFromString(const std::string& currency);
    static com::liversedge::messages::SettlType::Value settlTypeFromString(const std::string& settlType);
    static com::liversedge::messages::SecurityType::Value securityTypeFromString(const std::string& securityType);
    static com::liversedge::messages::OrdStatus::Value ordStatusFromFix(FIX::OrdStatus ordStatus);
    static com::liversedge::messages::Side::Value sideFromFix(FIX::Side side);
    static com::liversedge::messages::OrdRejReason::Value ordRejReasonFromFix(FIX::OrdRejReason ordRejReason);
    // SBE -> FIX
    static FIX::Side convertSide(const com::liversedge::messages::Side::Value& sbeType);
    static FIX::OrdType convertOrderType(const com::liversedge::messages::OrderType::Value& sbeType);
    static FIX::TimeInForce convertTimeInForce(const com::liversedge::messages::TimeInForce::Value& sbeType);

private:
    // Private constructor - this is a utility class with only static methods
    SBEUtils() = default;
};