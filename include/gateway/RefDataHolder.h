#pragma once

#include <unordered_map>
#include <memory>
#include "SecurityInfo.h"
#include "../../generated/com_liversedge_messages/SecurityDefinition.h"

class RefDataHolder
{
public:
    RefDataHolder() = default;

    // Security definition processing
    void onSecurityDefinition(const com::liversedge::messages::SecurityDefinition& decoder, std::uint64_t timestamp);

    // Security lookup
    const SecurityInfo* getSecurityInfo(std::int32_t securityId) const;

private:
    std::unordered_map<std::int32_t, std::unique_ptr<SecurityInfo>> m_securities;
};