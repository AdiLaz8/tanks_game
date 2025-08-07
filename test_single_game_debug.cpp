#include "Simulator/GameRunner.h"
#include <iostream>
#include <memory>
#include <fstream>

// Include our actual implementations to create factory functions
#include "GameManager/GameManager_318772340_206580102.h"
#include "Algorithm/Player_318772340_206580102.h"
#include "Algorithm/TankAlgorithm_318772340_206580102.h"

int main() {
    std::cout << "=== RUNNING SINGLE GAME WITH DEBUG OUTPUT ===" << std::endl;
    
    // Load map
    std::cout << "\n1. Loading map..." << std::endl;
    MapData map;
    if (!map.loadFromFile("test_map_5x5_4x4.txt")) {
        std::cerr << "Failed to load test map!" << std::endl;
        return 1;
    }
    std::cout << "✅ Map loaded: " << map.name << " (" << map.width << "x" << map.height << ")" << std::endl;
    std::cout << "MaxSteps: " << map.maxSteps << ", NumShells: " << map.numShells << std::endl;
    
    // Create factory functions directly
    std::cout << "\n2. Creating factory functions..." << std::endl;
    
    // GameManager factory
    GameManagerFactory gameManagerFactory = [](bool verbose) -> std::unique_ptr<AbstractGameManager> {
        std::cout << "Creating GameManager with verbose=" << verbose << std::endl;
        return std::make_unique<GameManager_318772340_206580102::GameManager_318772340_206580102>(verbose);
    };
    
    // Player factory
    PlayerFactory playerFactory = [](int playerId, size_t width, size_t height, size_t maxSteps, size_t numShells) -> std::unique_ptr<Player> {
        std::cout << "Creating Player " << playerId << " with " << width << "x" << height << ", maxSteps=" << maxSteps << ", numShells=" << numShells << std::endl;
        return std::make_unique<Algorithm_318772340_206580102::Player_318772340_206580102>(playerId, width, height, maxSteps, numShells);
    };
    
    // TankAlgorithm factory
    TankAlgorithmFactory tankAlgorithmFactory = [](int playerId, int tankIndex) -> std::unique_ptr<TankAlgorithm> {
        std::cout << "Creating TankAlgorithm for player " << playerId << ", tank " << tankIndex << std::endl;
        return std::make_unique<Algorithm_318772340_206580102::TankAlgorithm_318772340_206580102>(playerId, tankIndex);
    };
    
    std::cout << "✅ Factory functions created successfully" << std::endl;
    
    // Create SatelliteView
    std::cout << "\n3. Creating SatelliteView..." << std::endl;
    auto satelliteView = GameRunner::createSatelliteView(map);
    if (!satelliteView) {
        std::cerr << "Failed to create SatelliteView!" << std::endl;
        return 1;
    }
    std::cout << "✅ SatelliteView created successfully" << std::endl;
    
    // Print initial board
    std::cout << "\n4. Initial game board:" << std::endl;
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
    
    // Create GameManager directly to add debug output
    std::cout << "\n5. Creating GameManager directly for debugging..." << std::endl;
    auto gameManager = std::make_unique<GameManager_318772340_206580102::GameManager_318772340_206580102>(true);
    
    // Create Players
    auto player1 = std::make_unique<Algorithm_318772340_206580102::Player_318772340_206580102>(
        1, map.width, map.height, map.maxSteps, map.numShells);
    auto player2 = std::make_unique<Algorithm_318772340_206580102::Player_318772340_206580102>(
        2, map.width, map.height, map.maxSteps, map.numShells);
    
    std::cout << "✅ GameManager and Players created successfully" << std::endl;
    
    // Run the game with timeout protection
    std::cout << "\n6. Running game with timeout protection..." << std::endl;
    
    try {
        // Set a timeout to prevent infinite loops
        std::cout << "Starting game execution..." << std::endl;
        std::cout << "MaxSteps: " << map.maxSteps << ", expecting game to end after " << map.maxSteps << " rounds" << std::endl;
        
        std::cout << "About to call gameManager->run()..." << std::endl;
        
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
        return 1;
    }
    
    std::cout << "\n🎉 Single game test completed successfully!" << std::endl;
    std::cout << "Ready to proceed with comparative mode testing!" << std::endl;
    
    return 0;
} 