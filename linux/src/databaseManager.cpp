#include "databaseManager.hpp"
#include <iostream>

DatabaseManager::DatabaseManager(const std::string& dbPath)
    : dbPath_(dbPath)
{
    if (sqlite3_open(dbPath_.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "Could not open/create database at " << dbPath_ << ": "
                  << sqlite3_errmsg(db_) << std::endl;
        db_ = nullptr;
    }
}

DatabaseManager::~DatabaseManager() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseManager::initSchema() {
    std::string schema = R"(
        CREATE TABLE IF NOT EXISTS files (
            file_name       TEXT PRIMARY KEY,
            location        TEXT NOT NULL,
            original_size   INTEGER NOT NULL,
            compressed_size INTEGER NOT NULL,
            disk_path       TEXT,
            last_access_ts  INTEGER
        );
    )";
    return exec(schema);
}

bool DatabaseManager::insertOrUpdateFile(const FileRecord& record) {
    // Upsert logic using SQLite’s INSERT OR REPLACE
    std::string sql = 
        "INSERT OR REPLACE INTO files "
        "(file_name, location, original_size, compressed_size, disk_path, last_access_ts) "
        "VALUES ('" + record.fileName + "', '" + record.location + "', "
        + std::to_string(record.originalSize) + ", "
        + std::to_string(record.compressedSize) + ", '"
        + record.diskPath + "', "
        + std::to_string(record.lastAccessTs) + ");";

    return exec(sql);
}

std::optional<FileRecord> DatabaseManager::getFileRecord(const std::string& fileName) {
    std::string sql = "SELECT file_name, location, original_size, compressed_size, disk_path, last_access_ts "
                      "FROM files WHERE file_name = '" + fileName + "';";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing SELECT: " << sqlite3_errmsg(db_) << std::endl;
        return std::nullopt;
    }

    FileRecord rec;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        rec.fileName        = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        rec.location        = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        rec.originalSize    = sqlite3_column_int64(stmt, 2);
        rec.compressedSize  = sqlite3_column_int64(stmt, 3);
        rec.diskPath        = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        rec.lastAccessTs    = sqlite3_column_int64(stmt, 5);
    } else {
        // Not found or error
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    sqlite3_finalize(stmt);
    return rec;
}

bool DatabaseManager::deleteFileRecord(const std::string& fileName) {
    std::string sql = "DELETE FROM files WHERE file_name = '" + fileName + "';";
    return exec(sql);
}

bool DatabaseManager::exec(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL Error: " << (errMsg ? errMsg : "unknown") << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}
