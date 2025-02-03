#pragma once

#include "configManager.hpp"
#include "databaseManager.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <cstdint>

class StorageManager {
public:
    // We'll pass references to the config & db managers directly
    StorageManager(const StorageConfig& storageConfig, DatabaseManager& dbManager);

    // Store file content
    std::string storeFile(const std::string& fileName, const std::vector<uint8_t>& data);

    // Retrieve file content
    std::optional<std::vector<uint8_t>> retrieveFile(const std::string& fileName);

private:
    // We can still keep an in-memory cache, if desired
    // but let's keep it simpler and rely on DB for metadata.
    // Possibly: std::unordered_map<std::string, std::vector<uint8_t>> ramStorage_;

    const StorageConfig storageConfig_;
    DatabaseManager& db_;

    // Helper: compress/decompress
    std::vector<uint8_t> compressData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> decompressData(const std::vector<uint8_t>& data);

    // Helper: encrypt/decrypt
    std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> decryptData(const std::vector<uint8_t>& data);

    // File I/O
    bool writeToDisk(const std::string& fullPath, const std::vector<uint8_t>& data);
    std::optional<std::vector<uint8_t>> readFromDisk(const std::string& fullPath);

    // In real code, we’d have tier management logic to decide
    // RAM vs. SSD vs. HDD, plus LRU or time-based eviction, etc.
};
