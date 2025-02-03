#pragma once

#include <string>

struct StorageConfig {
    size_t maxRamBytes;
    std::string ssdPath;
    std::string hddPath;
};

struct DatabaseConfig {
    std::string dbPath;
};

class ConfigManager {
public:
    ConfigManager(const std::string& configFilePath);

    // Accessors
    StorageConfig getStorageConfig() const { return storageConfig_; }
    DatabaseConfig getDatabaseConfig() const { return databaseConfig_; }

private:
    StorageConfig storageConfig_;
    DatabaseConfig databaseConfig_;

    void loadConfig(const std::string& configFilePath);
};