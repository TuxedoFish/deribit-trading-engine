#include "../../include/gateway/RefDataHolder.h"
#include "../../generated/com_liversedge_messages/ActionEnum.h"
#include <iostream>

void RefDataHolder::onSecurityDefinition(com::liversedge::messages::SecurityDefinition& decoder, std::uint64_t timestamp)
{
    auto action = decoder.action();
    std::int32_t securityId = decoder.id();

    if (action == com::liversedge::messages::ActionEnum::ADD)
    {
        auto securityInfo = std::make_unique<SecurityInfo>(decoder);
        std::cout << "Added security: " << securityInfo->toString() << std::endl;
        m_securities[securityId] = std::move(securityInfo);
    }
    else if (action == com::liversedge::messages::ActionEnum::REMOVE)
    {
        m_securities.erase(securityId);
    }
}

const SecurityInfo* RefDataHolder::getSecurityInfo(std::int32_t securityId) const
{
    auto it = m_securities.find(securityId);
    return (it != m_securities.end()) ? it->second.get() : nullptr;
}