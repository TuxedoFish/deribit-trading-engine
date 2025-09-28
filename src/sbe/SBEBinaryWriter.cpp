#include "../../include/sbe/SBEBinaryWriter.h"

SBEBinaryWriter::SBEBinaryWriter()
    : messageCount_(0), buffer_(BUFFER_SIZE) {
}

SBEBinaryWriter::~SBEBinaryWriter() {
    close();
}

void SBEBinaryWriter::openNewFile(const std::string& filename, bool append) {
    close();
    filename_ = filename;

    // Create directories if they don't exist
    boost::filesystem::path filepath(filename);
    if (filepath.has_parent_path()) {
        boost::filesystem::create_directories(filepath.parent_path());
    }

    std::ios::openmode mode = std::ios::binary;
    if (append) {
        mode |= std::ios::app;
    } else {
        mode |= std::ios::trunc;
    }

    file_.open(filename, mode);
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename_);
    }
    std::cout << (append ? "Opened" : "Created") << " binary file: " << filename_ << std::endl;
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