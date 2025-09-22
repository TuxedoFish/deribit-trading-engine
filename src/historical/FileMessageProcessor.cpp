#include "../../include/historical/FileMessageProcessor.h"

FileMessageProcessor::FileMessageProcessor(const std::string& dataDictionaryFilePath, MessageProcessor& messageProcessor, SBEBinaryWriter& writer) :
    m_dataDictionary(dataDictionaryFilePath), m_writer(writer), m_processor(messageProcessor) {
}

void FileMessageProcessor::process(std::string msgStr) {
    try
    {
        FIX44::Message msg = FIX::Message(msgStr, m_dataDictionary, false);

        if (!m_sessionInitialized)
        {
            // Extract the three required fields
            std::string beginString = msg.getHeader().getField(FIX::FIELD::BeginString);
            std::string senderCompID = msg.getHeader().getField(FIX::FIELD::SenderCompID);
            std::string targetCompID = msg.getHeader().getField(FIX::FIELD::TargetCompID);

            // Create session ID
            m_sessionID = FIX::SessionID(beginString, senderCompID, targetCompID);
            m_sessionInitialized = true;
        }

        std::string msgType = msg.getHeader().getField(FIX::FIELD::MsgType);
        if (msgType == FIX::MsgType_SecurityList)
        {
            m_processor.onMessage(FIX44::SecurityList(msg), m_sessionID);
        } else if (msgType == FIX::MsgType_Logon)
        {
            m_processor.onMessage(FIX44::Logon(msg), m_sessionID);
        } else if (msgType == FIX::MsgType_Logout)
        {
            m_processor.onMessage(FIX44::Logout(msg), m_sessionID);
        } else if (msgType == FIX::MsgType_MarketDataIncrementalRefresh)
        {
            m_processor.onMessage(FIX44::MarketDataIncrementalRefresh(msg), m_sessionID);
        } else if (msgType == FIX::MsgType_MarketDataSnapshotFullRefresh)
        {
            m_processor.onMessage(FIX44::MarketDataSnapshotFullRefresh(msg), m_sessionID);
        }
    } catch (const FIX::InvalidMessage& e)
    {
        std::cerr << "Invalid FIX message: " << e.what() << std::endl;
        std::cerr << "Message string: " << msgStr << std::endl;
    } catch (const FIX::FieldNotFound& e)
    {
        std::cerr << "Invalid FIX message: " << e.what() << std::endl;
        std::cerr << "Message string: " << msgStr << std::endl;
    }
}

void FileMessageProcessor::nextFile(std::string filePath) {
    m_writer.openNewFile(filePath);
}