#include "../../include/marketdata/SBEBinaryWriter.h"

SBEBinaryWriter::SBEBinaryWriter()
    : messageCount_(0), buffer_(BUFFER_SIZE) {
}

SBEBinaryWriter::~SBEBinaryWriter() {
    close();
}

void SBEBinaryWriter::openNewFile(const std::string& filename) {
    close();
    filename_ = filename;

    // Create directories if they don't exist
    std::filesystem::path filepath(filename);
    if (filepath.has_parent_path()) {
        std::filesystem::create_directories(filepath.parent_path());
    }

    file_.open(filename, std::ios::binary | std::ios::trunc);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename_);
    }
    std::cout << "Created binary file: " << filename_ << std::endl;
}

// Write a SecurityDefinition message
bool SBEBinaryWriter::writeSecurityDefinition(const std::string& symbol) {
    try {
        // Clear buffer
        std::fill(buffer_.begin(), buffer_.end(), 0);

        // Create and encode message header
        com::liversedge::messages::MessageHeader hdr;
        com::liversedge::messages::SecurityDefinition secDef;

        hdr.wrap(buffer_.data(), 0, 0, buffer_.size())
            .blockLength(secDef.sbeBlockLength())
            .templateId(secDef.sbeTemplateId())
            .schemaId(secDef.sbeSchemaId())
            .version(secDef.sbeSchemaVersion());

        // Create and encode message body
        secDef.wrapForEncode(buffer_.data(), hdr.encodedLength(), buffer_.size());
        com::liversedge::messages::VarStringEncoding& symbolObj = secDef.symbol();
        symbolObj.length(static_cast<std::uint32_t>(symbol.length()));
        // Write string data AFTER the block length AND length field
        // Correct offset: header(8) + blockLength(4) + lengthField(4) = 16
        std::memcpy(buffer_.data() + hdr.encodedLength() + secDef.sbeBlockLength() + secDef.symbol().lengthEncodingLength(), symbol.c_str(), symbol.length());

        // Update the message position to account for variable data
        secDef.sbePosition(secDef.sbePosition() + 4 + symbol.length());

        // Calculate total message size correctly
        size_t totalSize = hdr.encodedLength() + secDef.sbeBlockLength() + 4 + symbol.length();

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
        std::cerr << "Error writing SecurityDefinition: " << e.what() << std::endl;
        return false;
    }
}

// Write multiple security definitions
bool SBEBinaryWriter::writeSecurityDefinitions(const std::vector<std::string>& symbols) {
    for (const auto& symbol : symbols) {
        if (!writeSecurityDefinition(symbol)) {
            return false;
        }
    }
    return true;
}

// Flush and close file
void SBEBinaryWriter::close() {
    if (file_.is_open()) {
        file_.flush();
        file_.close();
        std::cout << "Closed file " << filename_ << " after writing "
            << messageCount_ << " messages" << std::endl;
    }
}

// Get stats
size_t SBEBinaryWriter::getMessageCount() const { return messageCount_; }
const std::string& SBEBinaryWriter::getFilename() const { return filename_; }

// Check if file is open and ready
bool SBEBinaryWriter::isOpen() const { return file_.is_open(); }

// Force flush to disk
void SBEBinaryWriter::flush() {
    if (file_.is_open()) {
        file_.flush();
    }
}