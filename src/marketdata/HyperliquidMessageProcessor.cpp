#include "../../include/marketdata/HyperliquidMessageProcessor.h"
#include "../../include/sbe/SBEUtils.h"
#include <iostream>

HyperliquidMessageProcessor::HyperliquidMessageProcessor(SBEBinaryWriter& writer)
    : MessageProcessor(writer)
{
}

void HyperliquidMessageProcessor::onConnected()
{
    if (getConnectionStatus() == com::liversedge::messages::ConnectionStatusEnum::Value::ONLINE)
    {
        std::cerr << "Received connect while still online, invalidating the state" << std::endl;
        invalidateState(0);
    }

    updateConnectionStatus(com::liversedge::messages::ConnectionStatusEnum::STARTING, 0);
}

void HyperliquidMessageProcessor::onDisconnected()
{
    invalidateState(0);
}

void HyperliquidMessageProcessor::onMeta(const hyperliquid::MetaResponse& response)
{
    for (const auto& asset : response.universe)
    {
        int id = createSecurity(asset.name);

        if (m_shouldOutput)
        {
            if (!m_writer.prepareMessage(m_securityDefinition))
            {
                std::cerr << "Error preparing security definition for " << asset.name << std::endl;
                removeSecurity(id);
                continue;
            }

            // Metadata
            m_securityDefinition.id(id);
            m_securityDefinition.timestamp(0);
            m_securityDefinition.action(com::liversedge::messages::ActionEnum::ADD);

            m_securityDefinition.currency(com::liversedge::messages::Currency::USDC);
            m_securityDefinition.commCurrency(com::liversedge::messages::Currency::CONTRACT);
            m_securityDefinition.settlCurrency(com::liversedge::messages::Currency::USDC);

            // Every contract treated as a perpetual futures contract
            m_securityDefinition.securityType(com::liversedge::messages::SecurityType::FUTCO);
            m_securityDefinition.contractMultiplier().mantissa(SBEUtils::stringToMantissa("1", -8));
            m_securityDefinition.settlType(com::liversedge::messages::SettlType::REGULAR);
            m_securityDefinition.maturityDate().year(com::liversedge::messages::Date::yearNullValue())
                                               .month(com::liversedge::messages::Date::monthNullValue())
                                               .day(com::liversedge::messages::Date::dayNullValue());

            // Price precision: max decimal places = MAX_DECIMALS(6) - szDecimals
            int instrumentPricePrecision = 6 - asset.szDecimals;
            m_securityDefinition.instrumentPricePrecision(instrumentPricePrecision);
            m_securityDefinition.minPriceIncrement().mantissa(SBEUtils::powerOfTenMantissa(instrumentPricePrecision, -8));
            m_securityDefinition.minSizeIncrement().mantissa(SBEUtils::powerOfTenMantissa(asset.szDecimals, -4));

            // Variable length fields must be last
            SBEUtils::setVarString(m_securityDefinition, m_securityDefinition.symbol(), asset.name);

            if (!m_writer.writeMessage(m_securityDefinition))
            {
                std::cerr << "Error writing security definition for " << asset.name << std::endl;
                removeSecurity(id);
                continue;
            }
        }

        if (!updateSecurityStatus(id, 0, com::liversedge::messages::SecurityStatusEnum::Value::PENDING_SNAPSHOT))
        {
            std::cerr << "Error updating security status to PENDING_SNAPSHOT for " << asset.name << std::endl;
        }
    }

    updateConnectionStatus(com::liversedge::messages::ConnectionStatusEnum::Value::ONLINE, 0);
}

void HyperliquidMessageProcessor::onL2BookLevel(const hyperliquid::L2BookUpdate& book, const hyperliquid::PriceLevel& level)
{
    int securityId = getSecurityId(book.coin);
    if (securityId == -1)
    {
        return;
    }

    // Transition from PENDING_SNAPSHOT to ONLINE on first book level
    if (getSecurityStatus(securityId) == com::liversedge::messages::SecurityStatusEnum::Value::PENDING_SNAPSHOT)
    {
        updateSecurityStatus(securityId, book.time, com::liversedge::messages::SecurityStatusEnum::Value::ONLINE);
    }

    if (!m_shouldOutput)
    {
        return;
    }

    if (!m_writer.prepareMessage(m_mdUpdate))
    {
        std::cerr << "Error preparing MDUpdate message" << std::endl;
        return;
    }

    m_mdUpdate.securityId(securityId);
    m_mdUpdate.timestamp(book.time);
    m_mdUpdate.updateType(com::liversedge::messages::MDUpdateType::BOOK_UPDATE);
    m_mdUpdate.action(com::liversedge::messages::MDUpdateAction::Value::CHANGE);
    m_mdUpdate.side(level.side == hyperliquid::Side::Bid ? com::liversedge::messages::MDSide::BID : com::liversedge::messages::MDSide::ASK);
    SBEUtils::setPrice(m_mdUpdate.price(), level.px);
    SBEUtils::setQty(m_mdUpdate.qty(), level.sz);

    if (!m_writer.writeMessage(m_mdUpdate))
    {
        std::cerr << "Error writing MDUpdate message" << std::endl;
    }
}

void HyperliquidMessageProcessor::onTrade(const hyperliquid::Trade& trade)
{
    int securityId = getSecurityId(trade.coin);
    if (securityId == -1)
    {
        return;
    }

    if (!m_shouldOutput)
    {
        return;
    }

    if (!m_writer.prepareMessage(m_mdUpdate))
    {
        std::cerr << "Error preparing MDUpdate message" << std::endl;
        return;
    }

    m_mdUpdate.securityId(securityId);
    m_mdUpdate.timestamp(trade.time);
    m_mdUpdate.updateType(com::liversedge::messages::MDUpdateType::TRADE);
    m_mdUpdate.side(trade.side == 'B' ? com::liversedge::messages::MDSide::BID : com::liversedge::messages::MDSide::ASK);
    m_mdUpdate.action(com::liversedge::messages::MDUpdateAction::Value::NULL_VALUE);
    SBEUtils::setPrice(m_mdUpdate.price(), trade.px);
    SBEUtils::setQty(m_mdUpdate.qty(), trade.sz);
    m_mdUpdate.tradeId(trade.tid);

    if (!m_writer.writeMessage(m_mdUpdate))
    {
        std::cerr << "Error writing MDUpdate message" << std::endl;
    }
}
