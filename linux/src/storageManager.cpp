#include "storageManager.hpp"
#include <zlib.h>
#include <fstream>
#include <iostream>
#include <algorithm> // for std::reverse
#include <ctime>     // for time()

StorageManager::StorageManager(const StorageConfig& storageConfig, DatabaseManager& dbManager)
    : storageConfig_(storageConfig), db_(dbManager)
{
    // Possibly do some initialization. E.g., create the directories if needed.
}

// Store file
std::string StorageManager::storeFile(const std::string& fileName, const std::vector<uint8_t>& data) {
    // 1) Compress
    auto compressed = compressData(data);

    // 2) Encrypt
    auto encrypted = encryptData(compressed);
    size_t compEncSize = encrypted.size();

    // For demonstration, we’ll always store on SSD:
    std::string fullPath = storageConfig_.ssdPath + fileName;
    if (!writeToDisk(fullPath, encrypted)) {
        // fallback to HDD if SSD fails
        fullPath = storageConfig_.hddPath + fileName;
        writeToDisk(fullPath, encrypted);
    }

    // 3) Update database
    FileRecord record;
    record.fileName       = fileName;
    record.location       = "SSD"; // or "HDD" if fallback
    record.originalSize   = data.size();
    record.compressedSize = compEncSize;
    record.diskPath       = fullPath;
    record.lastAccessTs   = static_cast<long long>(std::time(nullptr));

    db_.insertOrUpdateFile(record);

    return fileName;
}

// Retrieve file
std::optional<std::vector<uint8_t>> StorageManager::retrieveFile(const std::string& fileName) {
    auto recordOpt = db_.getFileRecord(fileName);
    if (!recordOpt.has_value()) {
        return std::nullopt; // not in DB
    }
    auto record = recordOpt.value();

    // read from disk (or RAM if we had it there)
    auto encryptedOpt = readFromDisk(record.diskPath);
    if (!encryptedOpt.has_value()) {
        return std::nullopt;
    }

    auto encrypted  = encryptedOpt.value();
    auto compressed = decryptData(encrypted);
    auto original   = decompressData(compressed);

    // Update last access time in DB
    record.lastAccessTs = static_cast<long long>(std::time(nullptr));
    db_.insertOrUpdateFile(record);

    return original;
}

// Compress with zlib
std::vector<uint8_t> StorageManager::compressData(const std::vector<uint8_t>& data) {
    uLongf destSize = compressBound(data.size());
    std::vector<uint8_t> dest(destSize);
    int result = ::compress(dest.data(), &destSize, data.data(), data.size());
    if (result != Z_OK) {
        std::cerr << "Compression failed!\n";
        return {};
    }
    dest.resize(destSize);
    return dest;
}

// Decompress
std::vector<uint8_t> StorageManager::decompressData(const std::vector<uint8_t>& data) {
    // In real scenario, store the original size in the DB (record.originalSize).
    // Then allocate a buffer that large.
    // For demonstration, we’ll guess or store a big buffer.

    uLongf outSize = 1024 * 1024 * 50; // 50MB for example
    std::vector<uint8_t> outBuf(outSize);
    int result = ::uncompress(outBuf.data(), &outSize, data.data(), data.size());
    if (result != Z_OK) {
        std::cerr << "Decompression failed!\n";
        return {};
    }
    outBuf.resize(outSize);
    return outBuf;
}

// Encrypt (mock)
std::vector<uint8_t> StorageManager::encryptData(const std::vector<uint8_t>& data) {
    // Reverse as a placeholder
    std::vector<uint8_t> out = data;
    std::reverse(out.begin(), out.end());
    return out;
}

// Decrypt (mock)
std::vector<uint8_t> StorageManager::decryptData(const std::vector<uint8_t>& data) {
    // Reverse again
    std::vector<uint8_t> out = data;
    std::reverse(out.begin(), out.end());
    return out;
}

// Write to disk
bool StorageManager::writeToDisk(const std::string& fullPath, const std::vector<uint8_t>& data) {
    std::ofstream ofs(fullPath, std::ios::binary);
    if (!ofs.is_open()) {
        return false;
    }
    ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

// Read from disk
std::optional<std::vector<uint8_t>> StorageManager::readFromDisk(const std::string& fullPath) {
    std::ifstream ifs(fullPath, std::ios::binary | std::ios::ate);
    if (!ifs.is_open()) {
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
