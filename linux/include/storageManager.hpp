#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <optional>

// Example configuration for the capacities
struct StorageConfig {
    size_t maxRamBytes   = 1024 * 1024 * 100; // 100 MB
    std::string ssdPath  = "C:/MyData/SSD/";  // or /mnt/ssd/ in Linux
    std::string hddPath  = "D:/MyData/HDD/";  // or /mnt/hdd/ in Linux
};

class StorageManager {
public:
    explicit StorageManager(const StorageConfig& config);

    // Store file content; returns a file ID or handle
    std::string storeFile(const std::string& fileName, const std::vector<uint8_t>& data);

    // Retrieve the file’s content
    std::optional<std::vector<uint8_t>> retrieveFile(const std::string& fileName);

private:
    StorageConfig config_;

    // A simple structure to represent metadata about where the file is stored
    struct FileInfo {
        enum class Location {
            RAM,
            SSD,
            HDD
        };

        Location location;
        size_t originalSize;
        size_t compressedSize;
        std::string pathOnDisk; // valid if location != RAM
    };

    // Key: fileName, Value: file metadata
    std::unordered_map<std::string, FileInfo> fileIndex_;

    // RAM cache: store the *encrypted + compressed* bytes
    std::unordered_map<std::string, std::vector<uint8_t>> ramStorage_;
    size_t currentRamUsage_ = 0;

    // Helper functions
    std::vector<uint8_t> compressData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> decompressData(const std::vector<uint8_t>& data);

    // Mock encryption/decryption
    std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> decryptData(const std::vector<uint8_t>& data);

    // Tier management
    bool hasRamCapacity(size_t dataSize) const;
    void moveFromRamToSSD(const std::string& fileName);
    void moveFromSSDToHDD(const std::string& fileName);

    // File I/O
    bool writeToDisk(const std::string& fullPath, const std::vector<uint8_t>& data);
    std::optional<std::vector<uint8_t>> readFromDisk(const std::string& fullPath);
};

