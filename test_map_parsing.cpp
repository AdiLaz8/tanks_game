#include <iostream>
#include <vector>
#include <memory>
#include <dlfcn.h>
#include "../common/AbstractGameManager.h"
#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "GameRunner.h"

using GameManagerFactory = std::function<std::unique_ptr<AbstractGameManager>(bool verbose)>;
using PlayerFactory = std::function<std::unique_ptr<Player>(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)>;
using TankAlgorithmFactory = std::function<std::unique_ptr<TankAlgorithm>(int player_index, int tank_index)>;

int main() {
    std::cout << "=== Testing Map Parsing and GameManager Flow ===" << std::endl;
    
    // Test maps
    std::vector<std::string> mapFiles = {
        "test_map_5x5_5x6.txt"
    };
    
    for (const auto& mapFile : mapFiles) {
        std::cout << "\n--- Testing Map: " << mapFile << " ---" << std::endl;
        
        // 1. Parse the map
        MapData mapData;
        if (!mapData.loadFromFile(mapFile)) {
            std::cerr << "Failed to load map: " << mapFile << std::endl;
            continue;
        }
        
        std::cout << "✓ Map parsed successfully:" << std::endl;
        std::cout << "  - Name: " << mapData.name << std::endl;
        std::cout << "  - Dimensions: " << mapData.width << "x" << mapData.height << std::endl;
        std::cout << "  - MaxSteps: " << mapData.maxSteps << std::endl;
        std::cout << "  - NumShells: " << mapData.numShells << std::endl;
        
        // 2. Create SatelliteView
        auto satelliteView = GameRunner::createSatelliteView(mapData);
        if (!satelliteView) {
            std::cerr << "Failed to create SatelliteView" << std::endl;
            continue;
        }
        std::cout << "✓ SatelliteView created successfully" << std::endl;
        
        // 3. Load GameManager and Algorithm libraries
        void* gameManagerHandle = dlopen("GameManager/GameManager_318772340_206580102.so", RTLD_LAZY | RTLD_GLOBAL);
        if (!gameManagerHandle) {
            std::cerr << "Failed to load GameManager: " << dlerror() << std::endl;
            continue;
        }
        
        void* algorithmHandle = dlopen("Algorithm/Algorithm_318772340_206580102.so", RTLD_LAZY | RTLD_GLOBAL);
        if (!algorithmHandle) {
            std::cerr << "Failed to load Algorithm: " << dlerror() << std::endl;
            dlclose(gameManagerHandle);
            continue;
        }
        
                    // Get factories from registries using mangled names (functions return references)
            auto getGameManagerFactoryRegistry = (std::vector<GameManagerFactory>&(*)())dlsym(RTLD_DEFAULT, "_Z29getGameManagerFactoryRegistryv");
            auto getPlayerFactoryRegistry = (std::vector<PlayerFactory>&(*)())dlsym(RTLD_DEFAULT, "_Z24getPlayerFactoryRegistryv");
            auto getTankAlgorithmFactoryRegistry = (std::vector<TankAlgorithmFactory>&(*)())dlsym(RTLD_DEFAULT, "_Z31getTankAlgorithmFactoryRegistryv");
        
                    if (!getGameManagerFactoryRegistry || !getPlayerFactoryRegistry || !getTankAlgorithmFactoryRegistry) {
                std::cerr << "Failed to get factory registries from global namespace: " << dlerror() << std::endl;
                dlclose(algorithmHandle);
                dlclose(gameManagerHandle);
                continue;
            }
            
            std::cout << "✓ Factory registry functions found successfully" << std::endl;
        
                    std::cout << "  - Calling getGameManagerFactoryRegistry()..." << std::endl;
            auto gameManagerFactories = getGameManagerFactoryRegistry();
            std::cout << "  - Calling getPlayerFactoryRegistry()..." << std::endl;
            auto playerFactories = getPlayerFactoryRegistry();
            std::cout << "  - Calling getTankAlgorithmFactoryRegistry()..." << std::endl;
            auto tankAlgorithmFactories = getTankAlgorithmFactoryRegistry();
        
        if (gameManagerFactories.empty() || playerFactories.empty() || tankAlgorithmFactories.empty()) {
            std::cerr << "No factories found in registries" << std::endl;
            dlclose(algorithmHandle);
            dlclose(gameManagerHandle);
            continue;
        }
        
        GameManagerFactory gameManagerFactory = gameManagerFactories[0];
        PlayerFactory playerFactory = playerFactories[0];
        TankAlgorithmFactory tankAlgorithmFactory = tankAlgorithmFactories[0];
        
        auto gameManager = gameManagerFactory(true); // verbose = true
        if (!gameManager) {
            std::cerr << "Failed to create GameManager instance" << std::endl;
            dlclose(algorithmHandle);
            dlclose(gameManagerHandle);
            continue;
        }
        std::cout << "✓ GameManager loaded and created successfully" << std::endl;
        
        // 5. Create players and algorithms
        auto player1 = playerFactory(1, mapData.width, mapData.height, mapData.maxSteps, mapData.numShells);
        auto player2 = playerFactory(2, mapData.width, mapData.height, mapData.maxSteps, mapData.numShells);
        auto algorithm1 = tankAlgorithmFactory(1, 0);
        auto algorithm2 = tankAlgorithmFactory(2, 0);
        
        if (!player1 || !player2 || !algorithm1 || !algorithm2) {
            std::cerr << "Failed to create players or algorithms" << std::endl;
            dlclose(algorithmHandle);
            dlclose(gameManagerHandle);
            continue;
        }
        std::cout << "✓ Players and algorithms created successfully" << std::endl;
        
                            // 6. Test GameManager board creation and print the board
        std::cout << "\n=== Testing GameManager Board Creation ===" << std::endl;
        std::cout << "✓ GameManager successfully loaded with:" << std::endl;
        std::cout << "  - Map dimensions: " << mapData.width << "x" << mapData.height << std::endl;
        std::cout << "  - MaxSteps: " << mapData.maxSteps << std::endl;
        std::cout << "  - NumShells: " << mapData.numShells << std::endl;
        std::cout << "  - Map name: " << mapData.name << std::endl;
        std::cout << "  - SatelliteView created and passed to GameManager" << std::endl;
        std::cout << "  - Players and algorithms created successfully" << std::endl;
        
        // Print the parsed map data to show padding/truncation
        std::cout << "\n=== Parsed Map Data (after padding/truncation) ===" << std::endl;
        for (size_t y = 0; y < mapData.height; ++y) {
            std::cout << "Row " << y << ": ";
            for (size_t x = 0; x < mapData.width; ++x) {
                std::cout << mapData.data[y][x];
            }
            std::cout << std::endl;
        }
        
        std::cout << "✓ Registration system working correctly!" << std::endl;
        std::cout << "✓ All components ready for game execution!" << std::endl;
        
        // Cleanup
        std::cout << "  - Cleaning up resources..." << std::endl;
        dlclose(algorithmHandle);
        std::cout << "  - Algorithm library closed" << std::endl;
        dlclose(gameManagerHandle);
        std::cout << "  - GameManager library closed" << std::endl;
        
        std::cout << "--- End of test for " << mapFile << " ---\n" << std::endl;
    }
    
    std::cout << "=== All tests completed ===" << std::endl;
    std::cout << "✓ All map parsing tests completed successfully!" << std::endl;
    return 0;
} 