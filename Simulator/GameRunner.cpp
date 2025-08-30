#include "GameRunner.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <unistd.h>  // for getcwd, chdir
#include <cstdlib>   // for free
// Logger removed - using std::cout for output instead

// Helper function to parse key-value pairs like "MaxSteps = 100"
bool parseKeyValue(const std::string& line, const std::string& key, size_t& value) {
    size_t keyPos = line.find(key);
    if (keyPos == std::string::npos) {
        return false;
    }
    
    // Find the equals sign after the key
    size_t equalsPos = line.find('=', keyPos + key.length());
    if (equalsPos == std::string::npos) {
        return false;
    }
    
    // Extract the value part after the equals sign
    std::string valuePart = line.substr(equalsPos + 1);
    
    // Remove leading and trailing whitespace
    valuePart.erase(0, valuePart.find_first_not_of(" \t"));
    valuePart.erase(valuePart.find_last_not_of(" \t") + 1);
    
    // Try to parse the number
    try {
        value = std::stoull(valuePart);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool MapData::loadFromFile(const std::string& filepath, bool verbose) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open map file: " << filepath << std::endl;
        return false;
    }
    
    // Extract map name from filename for error file
    std::string mapName = filepath.substr(filepath.find_last_of("/\\") + 1);
    std::string errorFile = mapName + ".error";
    std::ofstream errorOut;
    // Only create error file if verbose mode is enabled
    if (verbose) {
        errorOut.open(errorFile);
    }
    
    filename = filepath;
    
    std::string line;
    
    // Line 1: Map name/description
    if (!std::getline(file, line)) {
        std::cerr << "Error: Cannot read map name from " << filepath << std::endl;
        if (errorOut.is_open()) {
            errorOut << "Error: Cannot read map name from " << filepath << std::endl;
        }
        return false;
    }
    name = line;
    
    // Line 2: MaxSteps = <NUM>
    if (!std::getline(file, line)) {
        std::cerr << "Error: Cannot read MaxSteps from " << filepath << std::endl;
        return false;
    }
    if (!parseKeyValue(line, "MaxSteps", maxSteps)) {
        std::cerr << "Error: Invalid MaxSteps format in " << filepath << ": " << line << std::endl;
        if (errorOut.is_open()) {
            errorOut << "Error: Invalid MaxSteps format in " << filepath << ": " << line << std::endl;
        }
        return false;
    }
    
    // Line 3: NumShells = <NUM>
    if (!std::getline(file, line)) {
        std::cerr << "Error: Cannot read NumShells from " << filepath << std::endl;
        return false;
    }
    if (!parseKeyValue(line, "NumShells", numShells)) {
        std::cerr << "Error: Invalid NumShells format in " << filepath << ": " << line << std::endl;
        return false;
    }
    
    // Line 4: Rows = <NUM>
    if (!std::getline(file, line)) {
        std::cerr << "Error: Cannot read Rows from " << filepath << std::endl;
        return false;
    }
    if (!parseKeyValue(line, "Rows", height)) {
        std::cerr << "Error: Invalid Rows format in " << filepath << ": " << line << std::endl;
        return false;
    }
    
    // Line 5: Cols = <NUM>
    if (!std::getline(file, line)) {
        std::cerr << "Error: Cannot read Cols from " << filepath << std::endl;
        return false;
    }
    if (!parseKeyValue(line, "Cols", width)) {
        std::cerr << "Error: Invalid Cols format in " << filepath << ": " << line << std::endl;
        return false;
    }
    
    // Read map data 
    data.resize(height);
    size_t actualRows = 0;
    size_t maxActualCols = 0;
    
    // First pass: read all available lines and find actual dimensions
    std::vector<std::string> rawLines;
    while (std::getline(file, line)) {
        // Trim trailing spaces to get actual content length
        size_t contentLength = line.find_last_not_of(" \t");
        if (contentLength != std::string::npos) {
            contentLength++; 
        } else {
            contentLength = 0; // Line is all spaces
        }
        
        rawLines.push_back(line);
        maxActualCols = std::max(maxActualCols, contentLength);
    }
    actualRows = rawLines.size();
    
    // Log any discrepancies
    if (actualRows < height) {
        std::string msg = "Warning: Map has " + std::to_string(actualRows) + " rows but expected " + std::to_string(height) + ". Adding " + std::to_string(height - actualRows) + " empty rows.";
        std::cout << msg << std::endl;
        if (errorOut.is_open()) {
            errorOut << msg << std::endl;
        }
    } else if (actualRows > height) {
        std::string msg = "Warning: Map has " + std::to_string(actualRows) + " rows but expected " + std::to_string(height) + ". Ignoring last " + std::to_string(actualRows - height) + " rows.";
        std::cout << msg << std::endl;
        if (errorOut.is_open()) {
            errorOut << msg << std::endl;
        }
    }
    
    if (maxActualCols < width) {
        std::string msg = "Warning: Map has " + std::to_string(maxActualCols) + " columns but expected " + std::to_string(width) + ". Adding " + std::to_string(width - maxActualCols) + " empty columns.";
        std::cout << msg << std::endl;
        if (errorOut.is_open()) {
            errorOut << msg << std::endl;
        }
    } else if (maxActualCols > width) {
        std::string msg = "Warning: Map has " + std::to_string(maxActualCols) + " columns but expected " + std::to_string(width) + ". Ignoring last " + std::to_string(maxActualCols - width) + " columns.";
        std::cout << msg << std::endl;
        if (errorOut.is_open()) {
            errorOut << msg << std::endl;
        }
    }
    
    // Second pass: build the final map with proper dimensions
    for (size_t y = 0; y < height; ++y) {
        data[y].resize(width, ' ');
        for (size_t x = 0; x < width; ++x) {
            if (y < actualRows && x < rawLines[y].length()) {
                data[y][x] = rawLines[y][x];
            } else {
                // Fill missing rows/columns with spaces
                data[y][x] = ' ';
            }
        }
    }
    
    std::cout << "Loaded map: " << name << " (" << width << "x" << height << "), MaxSteps=" << maxSteps << ", NumShells=" << numShells << std::endl;
    return true;
}

GameExecution GameRunner::runSingleGame(
    GameManagerFactory gameManagerFactory,
    const std::string& gameManagerName,
    PlayerFactory playerFactory,
    TankAlgorithmFactory tankAlgorithmFactory,
    const std::string& algorithm1Name,
    const std::string& algorithm2Name,
    const MapData& map,
    bool verbose) {
    

    
    GameExecution execution;
    execution.gameManagerName = gameManagerName;
    execution.algorithmName = algorithm1Name;
    execution.mapName = map.name;
    execution.player1Name = "Player1";
    execution.player2Name = "Player2";
    
    // Log removed
    // Logs removed
    
    auto startTime = std::chrono::steady_clock::now();
    
    try {
        // Create GameManager
        // Log removed
        auto gameManager = gameManagerFactory(verbose);
        if (!gameManager) {
            // Error removed
            throw std::runtime_error("Failed to create GameManager");
        }
        // Log removed: "GameManager created successfully" << std::endl;
        
        // Create Players 
        // Log removed: "Creating Player 1" << std::endl;
        auto player1 = playerFactory(1, map.width, map.height, map.maxSteps, map.numShells);
        // Log removed: "Creating Player 2" << std::endl;
        auto player2 = playerFactory(2, map.width, map.height, map.maxSteps, map.numShells);
        if (!player1 || !player2) {
            // Error removed
            throw std::runtime_error("Failed to create Players");
        }
        // Log removed: "Both players created successfully" << std::endl;
        
        // Create SatelliteView from map
        // Log removed: "Creating SatelliteView from map" << std::endl;
        auto satelliteView = createSatelliteView(map);
        if (!satelliteView) {
            // Error removed
            throw std::runtime_error("Failed to create SatelliteView");
        }
        // Log removed: "SatelliteView created successfully" << std::endl;
        
        // Run the game
        // Log removed: "=== STARTING GAME SIMULATION ===" << std::endl;
        std::cout << "[INFO] Calling GameManager->run() with parameters:" << std::endl;
        std::cout << "[DEBUG] Map size: " << map.width << "x" << map.height << std::endl;
        std::cout << "[DEBUG] Map name: " << map.name << std::endl;
        std::cout << "[DEBUG] Max steps: " << map.maxSteps << std::endl;
        std::cout << "[DEBUG] Shells per tank: " << map.numShells << std::endl;
        // Log removed: "  Player 1 algorithm: " + algorithm1Name << std::endl;
        // Log removed: "  Player 2 algorithm: " + algorithm2Name << std::endl;
        
        execution.result = gameManager->run(
            map.width, map.height,
            *satelliteView,
            map.name,
            map.maxSteps, map.numShells,
            *player1, algorithm1Name,  
            *player2, algorithm2Name,  
            tankAlgorithmFactory,
            tankAlgorithmFactory
        );
        
        // Log removed: "=== GAME SIMULATION COMPLETED ===" << std::endl;
        
        // Log detailed game results
        // Log removed: "=== DETAILED GAME RESULTS ===" << std::endl;
        // Log removed: "Winner: " + (execution.result.winner == 0 ? "TIE" : "Player " + std::to_string(execution.result.winner)) << std::endl;
        // Log removed: "Total Rounds Played: " + std::to_string(execution.result.rounds) << std::endl;
        
        std::string reasonStr;
        switch (execution.result.reason) {
            case GameResult::ALL_TANKS_DEAD: reasonStr = "ALL_TANKS_DEAD"; break;
            case GameResult::MAX_STEPS: reasonStr = "MAX_STEPS_REACHED"; break;
            case GameResult::ZERO_SHELLS: reasonStr = "ZERO_SHELLS"; break;
            default: reasonStr = "UNKNOWN"; break;
        }
        // Log removed: "End Reason: " + reasonStr << std::endl;
        
        // Log remaining tanks for each player
        if (execution.result.remaining_tanks.size() >= 2) {
            // Log removed: "Player 1 remaining tanks: " + std::to_string(execution.result.remaining_tanks[0]) << std::endl;
            // Log removed: "Player 2 remaining tanks: " + std::to_string(execution.result.remaining_tanks[1]) << std::endl;
        }
        
        // Log game state information if available
        if (execution.result.gameState) {
            // Log removed: "Final game state captured successfully" << std::endl;
        } else {
            // Log removed: "No final game state available" << std::endl;
        }
        
        execution.success = true;
        // Log removed: "Game execution marked as successful" << std::endl;
        
    } catch (const std::exception& e) {
        execution.success = false;
        execution.errorMessage = e.what();
        // Error removed
        std::cerr << "Game execution failed: " << e.what() << std::endl;
    } catch (...) {
        execution.success = false;
        execution.errorMessage = "Unknown error occurred";
        // Error removed
        std::cerr << "Game execution failed with unknown error" << std::endl;
    }
    
    auto endTime = std::chrono::steady_clock::now();
    execution.executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Log removed: "=== GAME EXECUTION SUMMARY ===" << std::endl;
    // Log removed: "Execution time: " + std::to_string(execution.executionTime.count()) + " ms" << std::endl;
    // Log removed: "Success: " + std::string(execution.success ? "true" : "false") << std::endl;
    if (!execution.success) {
        // Error removed
    }
    // Log removed: "=== END GAME EXECUTION ===" << std::endl;
    
    return execution;
}

std::unique_ptr<MapSatelliteView> GameRunner::createSatelliteView(const MapData& map) {
    return std::make_unique<MapSatelliteView>(map.data);
}

// MapSatelliteView implementation
MapSatelliteView::MapSatelliteView(const std::vector<std::vector<char>>& data) 
    : mapData(data), width(data.empty() ? 0 : data[0].size()), height(data.size()) {
}

char MapSatelliteView::getObjectAt(size_t x, size_t y) const {
    if (y >= height || x >= width) {
        return '&'; // Out of bounds
    }
    return mapData[y][x];
}