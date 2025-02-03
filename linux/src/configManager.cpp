#include "configManager.hpp"
#include <fstream>
#include <nlohmann/json.hpp> // JSON library

ConfigManager::ConfigManager(const std::string& configFilePath) {
    loadConfig(configFilePath);
}

void ConfigManager::loadConfig(const std::string& configFilePath) {
    std::ifstream ifs(configFilePath);
    if (!ifs.is_open()) {
        throw std::runtime_error("Could not open config file: " + configFilePath);
    }
    nlohmann::json j;
    ifs >> j;

    // Parse the JSON
    storageConfig_.maxRamBytes = j["storage"]["maxRamBytes"].get<size_t>();
    storageConfig_.ssdPath     = j["storage"]["ssdPath"].get<std::string>();
    storageConfig_.hddPath     = j["storage"]["hddPath"].get<std::string>();

    databaseConfig_.dbPath = j["database"]["dbPath"].get<std::string>();
}