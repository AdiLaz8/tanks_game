#include "Simulator/GameRunner.h"
#include <iostream>
#include <memory>
#include <dlfcn.h>
#include <fstream>

// Function pointer types for factory registries
typedef std::vector<GameManagerFactory>& (*GameManagerRegistryFunc)();
typedef std::vector<PlayerFactory>& (*PlayerRegistryFunc)();
typedef std::vector<TankAlgorithmFactory>& (*TankAlgorithmRegistryFunc)();

int main(int argc, char* argv[]) {
    std::cout << "=== RUNNING SIMULATOR WITH DYNAMIC LOADING ===" << std::endl;
    
    // Get map file from command line argument or use default
    std::string mapFile = "test_map_5x5_4x4.txt";
    if (argc > 1) {
        mapFile = argv[1];
    }
    
    // Load map
    std::cout << "\n1. Loading map..." << std::endl;
    MapData map;
    if (!map.loadFromFile(mapFile)) {
        std::cerr << "Failed to load test map: " << mapFile << std::endl;
        return 1;
    }
    std::cout << "✅ Map loaded: " << map.name << " (" << map.width << "x" << map.height << ")" << std::endl;
    
    // Load GameManager library
    std::cout << "\n2. Loading GameManager library..." << std::endl;
    void* gameManagerHandle = dlopen("GameManager/GameManager_318772340_206580102.so", RTLD_LAZY);
    if (!gameManagerHandle) {
        std::cerr << "Failed to load GameManager library: " << dlerror() << std::endl;
        return 1;
    }
    std::cout << "✅ GameManager library loaded successfully" << std::endl;
    
    // Load Algorithm library
    std::cout << "\n3. Loading Algorithm library..." << std::endl;
    void* algorithmHandle = dlopen("Algorithm/Algorithm_318772340_206580102.so", RTLD_LAZY);
    if (!algorithmHandle) {
        std::cerr << "Failed to load Algorithm library: " << dlerror() << std::endl;
        dlclose(gameManagerHandle);
        return 1;
    }
    std::cout << "✅ Algorithm library loaded successfully" << std::endl;
    
    // Get factory registry functions
    std::cout << "\n4. Getting factory registry functions..." << std::endl;
    
    // GameManager factory registry (using mangled C++ names)
    GameManagerRegistryFunc getGameManagerRegistry = (GameManagerRegistryFunc)dlsym(gameManagerHandle, "__Z29getGameManagerFactoryRegistryv");
    if (!getGameManagerRegistry) {
        std::cerr << "Failed to get GameManager registry function: " << dlerror() << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    // Player factory registry (using mangled C++ names)
    PlayerRegistryFunc getPlayerRegistry = (PlayerRegistryFunc)dlsym(algorithmHandle, "__Z24getPlayerFactoryRegistryv");
    if (!getPlayerRegistry) {
        std::cerr << "Failed to get Player registry function: " << dlerror() << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    // TankAlgorithm factory registry (using mangled C++ names)
    TankAlgorithmRegistryFunc getTankAlgorithmRegistry = (TankAlgorithmRegistryFunc)dlsym(algorithmHandle, "__Z31getTankAlgorithmFactoryRegistryv");
    if (!getTankAlgorithmRegistry) {
        std::cerr << "Failed to get TankAlgorithm registry function: " << dlerror() << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    std::cout << "✅ All factory registry functions obtained successfully" << std::endl;
    
    // Get the registries
    std::cout << "\n5. Getting factory registries..." << std::endl;
    auto& gameManagerRegistry = getGameManagerRegistry();
    auto& playerRegistry = getPlayerRegistry();
    auto& tankAlgorithmRegistry = getTankAlgorithmRegistry();
    
    std::cout << "GameManager factories: " << gameManagerRegistry.size() << std::endl;
    std::cout << "Player factories: " << playerRegistry.size() << std::endl;
    std::cout << "TankAlgorithm factories: " << tankAlgorithmRegistry.size() << std::endl;
    
    if (gameManagerRegistry.empty() || playerRegistry.empty() || tankAlgorithmRegistry.empty()) {
        std::cerr << "❌ One or more registries are empty!" << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    std::cout << "✅ All registries have factories" << std::endl;
    
    // Create SatelliteView
    std::cout << "\n6. Creating SatelliteView..." << std::endl;
    auto satelliteView = GameRunner::createSatelliteView(map);
    if (!satelliteView) {
        std::cerr << "Failed to create SatelliteView!" << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    std::cout << "✅ SatelliteView created successfully" << std::endl;
    
    // Print initial board
    std::cout << "\n7. Initial game board:" << std::endl;
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
    
    // Run the game using the first factory from each registry
    std::cout << "\n8. Running game with dynamic factories..." << std::endl;
    std::cout << "DEBUG: About to call GameRunner::runSingleGame..." << std::endl;
    std::cout << "DEBUG: gameManagerRegistry[0] address: " << &gameManagerRegistry[0] << std::endl;
    std::cout << "DEBUG: playerRegistry[0] address: " << &playerRegistry[0] << std::endl;
    std::cout << "DEBUG: tankAlgorithmRegistry[0] address: " << &tankAlgorithmRegistry[0] << std::endl;
    
    try {
        std::cout << "DEBUG: Calling GameRunner::runSingleGame..." << std::endl;
        GameExecution execution = GameRunner::runSingleGame(
            gameManagerRegistry[0],  // Use first GameManager factory
            "GameManager_318772340_206580102",
            playerRegistry[0],       // Use first Player factory
            tankAlgorithmRegistry[0], // Use first TankAlgorithm factory
            "TankAlgorithm_318772340_206580102",
            map,
            true  // verbose mode
        );
        std::cout << "DEBUG: GameRunner::runSingleGame returned successfully!" << std::endl;
        
        if (execution.success) {
            std::cout << "✅ Game completed successfully!" << std::endl;
            
            // Print game result
            std::cout << "\n=== GAME RESULT ===" << std::endl;
            if (execution.result.winner == 0) {
                std::cout << "Result: Tie" << std::endl;
            } else {
                std::cout << "Result: Player " << execution.result.winner << " wins" << std::endl;
            }
            std::cout << "Reason: " << execution.result.reason << std::endl;
            std::cout << "Rounds: " << execution.result.rounds << std::endl;
            std::cout << "Remaining tanks - Player 1: " << execution.result.remaining_tanks[0] 
                      << ", Player 2: " << execution.result.remaining_tanks[1] << std::endl;
            std::cout << "Execution time: " << execution.executionTime.count() << "ms" << std::endl;
            
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
            }
            
            std::cout << "\n🎉 Dynamic loading test completed successfully!" << std::endl;
            std::cout << "Ready to proceed with comparative mode testing!" << std::endl;
        } else {
            std::cerr << "❌ Game execution failed!" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Exception during game execution: " << e.what() << std::endl;
    }
    
    // Clean up
    std::cout << "\n9. Cleaning up..." << std::endl;
    dlclose(algorithmHandle);
    dlclose(gameManagerHandle);
    std::cout << "✅ Libraries unloaded successfully" << std::endl;
    
    return 0;
} 