#include <iostream>
#include <vector>
#include "configManager.hpp"
#include "databaseManager.hpp"
#include "storageManager.hpp"

int main() {
    try {
        // 1) Load config
        ConfigManager cfgMgr("config.json");
        auto storageCfg = cfgMgr.getStorageConfig();
        auto dbCfg      = cfgMgr.getDatabaseConfig();

        // 2) Initialize Database
        DatabaseManager dbManager(dbCfg.dbPath);
        dbManager.initSchema();

        // 3) Create StorageManager
        StorageManager storage(storageCfg, dbManager);

        // 4) Test storing a file
        std::string fileName = "hello.txt";
        std::vector<uint8_t> data = {'H','e','l','l','o',' ','D','a','t','a','b','a','s','e'};

        storage.storeFile(fileName, data);
        std::cout << "Stored file: " << fileName << std::endl;

        // 5) Retrieve the file
        auto retrieved = storage.retrieveFile(fileName);
        if (retrieved.has_value()) {
            std::string text(retrieved->begin(), retrieved->end());
            std::cout << "Retrieved content: " << text << std::endl;
        } else {
            std::cout << "Failed to retrieve file.\n";
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }

    return 0;
}
