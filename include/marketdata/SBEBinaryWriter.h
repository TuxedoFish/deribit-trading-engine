#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include "../../generated/com_liversedge_messages/MessageHeader.h"
#include "../../generated/com_liversedge_messages/SecurityDefinition.h"

class SBEBinaryWriter {
private:
    std::ofstream file_;
    std::string filename_;
    size_t messageCount_;
    std::vector<char> buffer_;

    static constexpr size_t BUFFER_SIZE = 1024; // Adjust based on max message size

public:
    SBEBinaryWriter();
    ~SBEBinaryWriter();

    bool writeSecurityDefinition(const std::string& symbol);
    bool writeSecurityDefinitions(const std::vector<std::string>& symbols);

    void openNewFile(const std::string& filename);
    void close();

    size_t getMessageCount() const;
    const std::string& getFilename() const;
    bool isOpen() const;
    void flush();
};