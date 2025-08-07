#include "Simulator/GameRunner.h"
#include <iostream>
#include <dlfcn.h>
#include <memory>
#include <fstream>

// Include the common headers for proper types
#include "common/AbstractGameManager.h"
#include "common/Player.h"
#include "common/TankAlgorithm.h"
#include "common/SatelliteView.h"
#include "common/GameResult.h"

int main() {
    std::cout << "=== TESTING COMPLETE GAME EXECUTION ===" << std::endl;
    
    // Test map parsing first
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
    
    // Cast to proper factory types
    auto gameManagerFactory = (GameManagerFactory)(*gameManagerFactories)[0];
    auto playerFactory = (PlayerFactory)(*playerFactories)[0];
    auto tankAlgorithmFactory = (TankAlgorithmFactory)(*tankAlgorithmFactories)[0];
    
    std::cout << "✅ Factories retrieved successfully" << std::endl;
    
    // Create instances
    std::cout << "\n3. Creating game instances..." << std::endl;
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
    std::cout << "\n4. Creating SatelliteView..." << std::endl;
    auto satelliteView = GameRunner::createSatelliteView(map);
    if (!satelliteView) {
        std::cerr << "Failed to create SatelliteView!" << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    std::cout << "✅ SatelliteView created successfully" << std::endl;
    
    // Print initial board
    std::cout << "\n5. Initial game board:" << std::endl;
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
    
    // Create output file for game results
    std::string outputFile = "output_test_game.txt";
    std::ofstream gameOutput(outputFile);
    if (!gameOutput.is_open()) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    std::cout << "\n6. Running game..." << std::endl;
    
    // Run the game using the GameManager's run method
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
        
        // Check if output file was created and has content
        gameOutput.close();
        std::ifstream checkOutput(outputFile);
        if (checkOutput.is_open()) {
            std::cout << "\n=== GAME OUTPUT FILE CONTENTS ===" << std::endl;
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
    
    std::cout << "\n🎉 Complete game test finished!" << std::endl;
    std::cout << "Output file: " << outputFile << std::endl;
    std::cout << "Ready to proceed with comparative mode testing!" << std::endl;
    
    return 0;
} 