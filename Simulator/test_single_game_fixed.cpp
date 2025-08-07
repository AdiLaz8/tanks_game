#include "GameRunner.h"
#include <iostream>
#include <dlfcn.h>
#include <memory>
#include <functional>
#include <fstream>

// Include the common headers for proper types
#include "../common/AbstractGameManager.h"
#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "../common/SatelliteView.h"
#include "../common/GameResult.h"

int main() {
    std::cout << "=== RUNNING SINGLE GAME WITH PROPER FACTORY CASTING ===" << std::endl;
    
    // Load map
    std::cout << "\n1. Loading map..." << std::endl;
    MapData map;
    if (!map.loadFromFile("test_map_5x5_4x4.txt")) {
        std::cerr << "Failed to load test map!" << std::endl;
        return 1;
    }
    std::cout << "✅ Map loaded: " << map.name << " (" << map.width << "x" << map.height << ")" << std::endl;
    
    // Load libraries
    std::cout << "\n2. Loading libraries..." << std::endl;
    void* gameManagerHandle = dlopen("GameManager/GameManager_318772340_206580102.so", RTLD_LAZY | RTLD_GLOBAL);
    void* algorithmHandle = dlopen("Algorithm/Algorithm_318772340_206580102.so", RTLD_LAZY | RTLD_GLOBAL);
    
    if (!gameManagerHandle || !algorithmHandle) {
        std::cerr << "Failed to load libraries: " << dlerror() << std::endl;
        return 1;
    }
    std::cout << "✅ Libraries loaded successfully" << std::endl;
    
    // Get registry functions
    auto getGameManagerRegistry = (std::vector<void*>*(*)())dlsym(gameManagerHandle, "getGameManagerFactoryRegistryC");
    auto getPlayerRegistry = (std::vector<void*>*(*)())dlsym(algorithmHandle, "getPlayerFactoryRegistryC");
    auto getTankAlgorithmRegistry = (std::vector<void*>*(*)())dlsym(algorithmHandle, "getTankAlgorithmFactoryRegistryC");
    
    if (!getGameManagerRegistry || !getPlayerRegistry || !getTankAlgorithmRegistry) {
        std::cerr << "Failed to get registry functions: " << dlerror() << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    // Get factories
    auto* gameManagerFactories = getGameManagerRegistry();
    auto* playerFactories = getPlayerRegistry();
    auto* tankAlgorithmFactories = getTankAlgorithmRegistry();
    
    if (gameManagerFactories->empty() || playerFactories->empty() || tankAlgorithmFactories->empty()) {
        std::cerr << "No factories found in registries!" << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    std::cout << "✅ Factories retrieved successfully" << std::endl;
    
    // Properly cast the void pointers to function types
    std::cout << "\n3. Casting factory functions..." << std::endl;
    
    // Cast to the actual function pointer types
    auto gameManagerFactoryPtr = reinterpret_cast<std::unique_ptr<AbstractGameManager>(*)(bool)>((*gameManagerFactories)[0]);
    auto playerFactoryPtr = reinterpret_cast<std::unique_ptr<Player>(*)(int, size_t, size_t, size_t, size_t)>((*playerFactories)[0]);
    auto tankAlgorithmFactoryPtr = reinterpret_cast<std::unique_ptr<TankAlgorithm>(*)(int, int)>((*tankAlgorithmFactories)[0]);
    
    // Create std::function wrappers
    GameManagerFactory gameManagerFactory = gameManagerFactoryPtr;
    PlayerFactory playerFactory = playerFactoryPtr;
    TankAlgorithmFactory tankAlgorithmFactory = tankAlgorithmFactoryPtr;
    
    std::cout << "✅ Factory functions cast successfully" << std::endl;
    
    // Create instances
    std::cout << "\n4. Creating game instances..." << std::endl;
    auto gameManager = gameManagerFactory(true); // verbose mode
    auto player1 = playerFactory(1, map.width, map.height, map.maxSteps, map.numShells);
    auto player2 = playerFactory(2, map.width, map.height, map.maxSteps, map.numShells);
    
    if (!gameManager || !player1 || !player2) {
        std::cerr << "Failed to create instances!" << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    std::cout << "✅ Game instances created successfully" << std::endl;
    
    // Create SatelliteView
    std::cout << "\n5. Creating SatelliteView..." << std::endl;
    auto satelliteView = GameRunner::createSatelliteView(map);
    if (!satelliteView) {
        std::cerr << "Failed to create SatelliteView!" << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    std::cout << "✅ SatelliteView created successfully" << std::endl;
    
    // Print initial board
    std::cout << "\n6. Initial game board:" << std::endl;
    for (size_t y = 0; y < map.height; ++y) {
        std::cout << "Row " << y << ": ";
        for (size_t x = 0; x < map.width; ++x) {
            char c = satelliteView->getObjectAt(x, y);
            if (c == ' ') {
                std::cout << '.';  // Make spaces visible
            } else {
                std::cout << c;
            }
        }
        std::cout << std::endl;
    }
    
    // Run the game
    std::cout << "\n7. Running game..." << std::endl;
    
    try {
        GameResult result = gameManager->run(
            map.width, map.height,
            *satelliteView,
            map.name,
            map.maxSteps, map.numShells,
            *player1, "Player1", *player2, "Player2",
            tankAlgorithmFactory, tankAlgorithmFactory
        );
        
        std::cout << "✅ Game completed successfully!" << std::endl;
        
        // Print game result
        std::cout << "\n=== GAME RESULT ===" << std::endl;
        if (result.winner == 0) {
            std::cout << "Result: Tie" << std::endl;
        } else {
            std::cout << "Result: Player " << result.winner << " wins" << std::endl;
        }
        std::cout << "Reason: " << result.reason << std::endl;
        std::cout << "Rounds: " << result.rounds << std::endl;
        std::cout << "Remaining tanks - Player 1: " << result.remaining_tanks[0] 
                  << ", Player 2: " << result.remaining_tanks[1] << std::endl;
        
        // Check for output file (EX2-style output)
        std::string outputFile = "output_" + map.name;
        std::ifstream checkOutput(outputFile);
        if (checkOutput.is_open()) {
            std::cout << "\n=== GAME OUTPUT FILE CONTENTS (EX2 Style) ===" << std::endl;
            std::cout << "File: " << outputFile << std::endl;
            std::string line;
            while (std::getline(checkOutput, line)) {
                std::cout << line << std::endl;
            }
            checkOutput.close();
        } else {
            std::cout << "Warning: No output file generated" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception during game execution: " << e.what() << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    // Cleanup
    dlclose(algorithmHandle);
    dlclose(gameManagerHandle);
    
    std::cout << "\n🎉 Single game test completed successfully!" << std::endl;
    std::cout << "Ready to proceed with comparative mode testing!" << std::endl;
    
    return 0;
} 