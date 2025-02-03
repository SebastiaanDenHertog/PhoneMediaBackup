#pragma once

#include <string>
#include <optional>
#include <sqlite3.h>

struct FileRecord {
    std::string fileName;
    std::string location;
    size_t originalSize;
    size_t compressedSize;
    std::string diskPath;
    long long lastAccessTs; 
};

class DatabaseManager {
public:
    DatabaseManager(const std::string& dbPath);
    ~DatabaseManager();

    bool initSchema();

    bool insertOrUpdateFile(const FileRecord& record);
    std::optional<FileRecord> getFileRecord(const std::string& fileName);
    bool deleteFileRecord(const std::string& fileName);

    // Additional queries as needed...

private:
    sqlite3* db_ = nullptr;
    std::string dbPath_;

    // Helper
    bool exec(const std::string& sql);
};
