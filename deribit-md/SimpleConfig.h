// SimpleConfig.h - Basic key-value configuration manager
#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

class SimpleConfig {
private:
    std::unordered_map<std::string, std::string> config_values_;
    std::string config_file_path_;

public:
    // Load configuration from file
    explicit SimpleConfig(const std::string& config_file_path)
        : config_file_path_(config_file_path) {
        loadFromFile();
    }

    std::string getString(const std::string& key) const;
    std::string getString(const std::string& key, const std::string& default_value) const;
    int getInt(const std::string& key) const;
    int getInt(const std::string& key, int default_value) const;
    double getDouble(const std::string& key) const;
    double getDouble(const std::string& key, double default_value) const;
    bool getBool(const std::string& key) const;
    bool getBool(const std::string& key, bool default_value) const;
    bool hasKey(const std::string& key) const;

    // Print all configuration values (for debugging)
    void printAll() const {
        std::cout << "Configuration from: " << config_file_path_ << std::endl;
        for (const auto& pair : config_values_) {
            std::cout << pair.first << " = " << pair.second << std::endl;
        }
    }

private:
    void loadFromFile();
    std::string trim(const std::string& str) const;
};
