#include "StorageManager.hpp"
#include <iostream>

int main() {
    StorageConfig config;
    config.maxRamBytes = 1024 * 1024 * 10;  // 10 MB limit in RAM
    config.ssdPath     = "C:/Temp/SSD/";   // Adjust to your system
    config.hddPath     = "D:/Temp/HDD/";

    // Create manager
    StorageManager storage(config);

    // Example usage
    std::string fileName = "example.txt";
    std::vector<uint8_t> fileData = {'H','e','l','l','o',' ','W','o','r','l','d'};

    // Store
    storage.storeFile(fileName, fileData);
    std::cout << "File stored in multi-tier storage system.\n";

    // Retrieve
    auto retrieved = storage.retrieveFile(fileName);
    if (retrieved.has_value()) {
        std::string text(retrieved->begin(), retrieved->end());
        std::cout << "Retrieved content: " << text << "\n";
    } else {
        std::cout << "Failed to retrieve file.\n";
    }

    return 0;
}
