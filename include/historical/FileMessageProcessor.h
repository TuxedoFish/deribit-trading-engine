#pragma once

#include <string>
#include <exception>
#include "quickfix/Application.h"
#include "../marketdata/SBEBinaryWriter.h"
#include "../marketdata/MessageProcessor.h"

class FileMessageProcessor
{
public:
    explicit FileMessageProcessor(const std::string&, MessageProcessor&, SBEBinaryWriter&);
    virtual ~FileMessageProcessor() = default;

    void process(std::string);
    void nextFile(std::string);

private:
    FIX::SessionID m_sessionID;
    bool m_sessionInitialized = false;
    FIX::DataDictionary m_dataDictionary;
    SBEBinaryWriter& m_writer;
    MessageProcessor& m_processor;
};
