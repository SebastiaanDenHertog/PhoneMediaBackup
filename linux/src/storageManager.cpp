#include "StorageManager.hpp"

#include <iostream>
#include <fstream>
#include <zlib.h>  // for compression/decompression

// ------------------- Constructor -------------------
StorageManager::StorageManager(const StorageConfig& config)
    : config_(config) {
    // Possibly create directories for SSD/HDD if they don't exist, etc.
}

// ------------------- Public: Store File -------------------
std::string StorageManager::storeFile(const std::string& fileName, const std::vector<uint8_t>& data) {
    // 1) Compress data
    auto compressed = compressData(data);

    // 2) Encrypt data
    auto encrypted = encryptData(compressed);
    size_t dataSize = encrypted.size();

    // 3) Decide if we can store it in RAM
    if (hasRamCapacity(dataSize)) {
        // Store in RAM
        ramStorage_[fileName] = encrypted;
        fileIndex_[fileName] = FileInfo {
            FileInfo::Location::RAM,
            data.size(),
            dataSize,
            "" // No disk path since it's in RAM
        };
        currentRamUsage_ += dataSize;
    } else {
        // Not enough RAM. We try SSD or go directly to HDD if needed.
        // For simplicity, let's just store to SSD if possible.

        // Construct the path on SSD
        std::string fullPath = config_.ssdPath + fileName;
        if (!writeToDisk(fullPath, encrypted)) {
            // If SSD write fails for some reason, fallback to HDD or handle error
            fullPath = config_.hddPath + fileName;
            writeToDisk(fullPath, encrypted);
            fileIndex_[fileName] = FileInfo {
                FileInfo::Location::HDD,
                data.size(),
                dataSize,
                fullPath
            };
        } else {
            fileIndex_[fileName] = FileInfo {
                FileInfo::Location::SSD,
                data.size(),
                dataSize,
                fullPath
            };
        }
    }

    return fileName;
}

// ------------------- Public: Retrieve File -------------------
std::optional<std::vector<uint8_t>> StorageManager::retrieveFile(const std::string& fileName) {
    // Check if in index
    auto it = fileIndex_.find(fileName);
    if (it == fileIndex_.end()) {
        return std::nullopt; // Not found
    }
    FileInfo& fileInfo = it->second;

    std::vector<uint8_t> encrypted;
    switch (fileInfo.location) {
        case FileInfo::Location::RAM: {
            encrypted = ramStorage_.at(fileName);
            break;
        }
        case FileInfo::Location::SSD:
        case FileInfo::Location::HDD: {
            auto diskData = readFromDisk(fileInfo.pathOnDisk);
            if (!diskData) {
                return std::nullopt; // Error reading from disk
            }
            encrypted = *diskData;
            break;
        }
    }

    // Decrypt
    auto compressed = decryptData(encrypted);
    // Decompress
    auto originalData = decompressData(compressed);

    return originalData;
}

// ------------------- Private: Compression -------------------
std::vector<uint8_t> StorageManager::compressData(const std::vector<uint8_t>& data) {
    // Using zlib’s compress function for simplicity

    // Worst-case size per zlib docs
    uLongf destSize = compressBound(data.size());
    std::vector<uint8_t> dest(destSize);

    int result = ::compress(dest.data(), &destSize, data.data(), data.size());
    if (result != Z_OK) {
        // handle error
        std::cerr << "Compression failed!\n";
        return {};
    }
    dest.resize(destSize);
    return dest;
}

// ------------------- Private: Decompression -------------------
std::vector<uint8_t> StorageManager::decompressData(const std::vector<uint8_t>& data) {
    // In real scenario, we’d need the original size or store it in a header
    // For this demo, let’s guess we have it in a header or metadata.

    // We'll assume we store the `originalSize` in fileInfo. 
    // This function would be more robust with real error checking.

    // For demonstration, let's do a naive approach:
    // We'll just try a large buffer to decompress into.

    uLongf outputSize = 1024 * 1024 * 200; // 200 MB max for demonstration
    std::vector<uint8_t> outBuffer(outputSize);

    int result = ::uncompress(outBuffer.data(), &outputSize, data.data(), data.size());
    if (result != Z_OK) {
        // handle error
        std::cerr << "Decompression failed!\n";
        return {};
    }
    outBuffer.resize(outputSize);
    return outBuffer;
}

// ------------------- Private: Mock Encryption/Decryption -------------------
std::vector<uint8_t> StorageManager::encryptData(const std::vector<uint8_t>& data) {
    // This is just a dummy “encryption” that reverses data.  
    // In real code, use an AES library like OpenSSL with a proper key & IV.

    std::vector<uint8_t> encrypted = data;
    std::reverse(encrypted.begin(), encrypted.end());
    return encrypted;
}

std::vector<uint8_t> StorageManager::decryptData(const std::vector<uint8_t>& data) {
    // Reverse again to “decrypt”.

    std::vector<uint8_t> decrypted = data;
    std::reverse(decrypted.begin(), decrypted.end());
    return decrypted;
}

// ------------------- Private: Tier Management -------------------
bool StorageManager::hasRamCapacity(size_t dataSize) const {
    return (currentRamUsage_ + dataSize) <= config_.maxRamBytes;
}

void StorageManager::moveFromRamToSSD(const std::string& fileName) {
    // In a real system, you’d decide which file in RAM to evict. 
    // Then compress, encrypt (already done in this example), and write to SSD.
}

void StorageManager::moveFromSSDToHDD(const std::string& fileName) {
    // Similar logic: read from SSD, write to HDD, update metadata, remove from SSD.
}

// ------------------- Private: File I/O -------------------
bool StorageManager::writeToDisk(const std::string& fullPath, const std::vector<uint8_t>& data) {
    std::ofstream ofs(fullPath, std::ios::binary);
    if (!ofs.is_open()) {
        std::cerr << "Could not open file for writing: " << fullPath << "\n";
        return false;
    }
    ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

std::optional<std::vector<uint8_t>> StorageManager::readFromDisk(const std::string& fullPath) {
    std::ifstream ifs(fullPath, std::ios::binary | std::ios::ate);
    if (!ifs.is_open()) {
        std::cerr << "Could not open file for reading: " << fullPath << "\n";
        return std::nullopt;
    }

    std::streamsize fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(fileSize);
    if (!ifs.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        return std::nullopt;
    }
    return buffer;
}
