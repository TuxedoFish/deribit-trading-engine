#include "../../include/historical/FileMessageProcessor.h"

FileMessageProcessor::FileMessageProcessor(const std::string& dataDictionaryFilePath, MessageProcessor& messageProcessor, SBEBinaryWriter& writer) :
    m_dataDictionary(dataDictionaryFilePath), m_writer(writer), m_processor(messageProcessor) {
}

void FileMessageProcessor::process(std::string msgStr) {
    FIX::Message msg(msgStr, m_dataDictionary, true);

    if (!m_sessionInitialized) {
        // Extract the three required fields
        std::string beginString = msg.getHeader().getField(FIX::FIELD::BeginString);
        std::string senderCompID = msg.getHeader().getField(FIX::FIELD::SenderCompID);
        std::string targetCompID = msg.getHeader().getField(FIX::FIELD::TargetCompID);

        // Create session ID
        m_sessionID = FIX::SessionID(beginString, senderCompID, targetCompID);
        m_sessionInitialized = true;
    }

    // Get message type
    std::string msgType = msg.getHeader().getField(FIX::FIELD::MsgType);

    // Create and crack the appropriate typed message
    if (msgType == "y") {  // SecurityList
        FIX44::SecurityList securityList{ msg };
        m_processor.onMessage(securityList, m_sessionID);
    }
}

void FileMessageProcessor::nextFile(std::string filePath) {
    m_writer.openNewFile(filePath);
}