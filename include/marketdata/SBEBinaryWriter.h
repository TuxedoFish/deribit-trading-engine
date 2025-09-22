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

    static constexpr size_t BUFFER_SIZE = 131072;

public:
    SBEBinaryWriter();
    ~SBEBinaryWriter();

    template<typename T>
    bool writeMessage(T& message);
    template<typename T>
    bool prepareMessage(T& message);

    void openNewFile(const std::string& filename);
    void close();

    size_t getMessageCount() const;
    const std::string& getFilename() const;
    bool isOpen() const;
    void flush();
};

// Generic write method that works with any SBE message type
template<typename T>
bool SBEBinaryWriter::prepareMessage(T& message) {
    try {
        // Clear buffer
        std::fill(buffer_.begin(), buffer_.end(), 0);

        // Create and encode message header
        com::liversedge::messages::MessageHeader hdr;
        const size_t headerSize = hdr.encodedLength();

        // Wrap header at the beginning of buffer
        hdr.wrap(buffer_.data(), 0, 0, buffer_.size())
            .blockLength(message.sbeBlockLength())
            .templateId(message.sbeTemplateId())
            .schemaId(message.sbeSchemaId())
            .version(message.sbeSchemaVersion());

        // Wrap message after the header
        message.wrapForEncode(buffer_.data(), headerSize, buffer_.size() - headerSize);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error writing message: " << e.what() << std::endl;
        return false;
    }
}

// Generic write method that works with any SBE message type
template<typename T>
bool SBEBinaryWriter::writeMessage(T& message) {
    try {
        // Calculate total encoded size (header + message payload)
        com::liversedge::messages::MessageHeader hdr;
        size_t totalSize = hdr.encodedLength() + message.encodedLength();

        // Ensure we don't exceed buffer size
        if (totalSize > buffer_.size()) {
            std::cerr << "Message too large for buffer. Size: " << totalSize
                << ", Buffer: " << buffer_.size() << std::endl;
            return false;
        }

        // Write to file
        file_.write(buffer_.data(), totalSize);
        if (!file_.good()) {
            std::cerr << "Error writing to file" << std::endl;
            return false;
        }

        messageCount_++;
        return true;

    }
    catch (const std::exception& e) {
        std::cerr << "Error writing message: " << e.what() << std::endl;
        return false;
    }
}

