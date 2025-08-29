#include "GameRunner.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

// Helper function to parse key-value pairs like "MaxSteps = 100"
bool parseKeyValue(const std::string& line, const std::string& key, size_t& value) {
    // Find the key
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
    name = line; // Store the actual map name from the file
    
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
    
    // Read map data (Lines 6 and on) with EX2 error handling
    data.resize(height);
    size_t actualRows = 0;
    size_t maxActualCols = 0;
    
    // First pass: read all available lines and find actual dimensions
    std::vector<std::string> rawLines;
    while (std::getline(file, line)) {
        // Trim trailing spaces to get actual content length
        size_t contentLength = line.find_last_not_of(" \t");
        if (contentLength != std::string::npos) {
            contentLength++; // Include the last non-space character
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
    
    std::cout << "DEBUG: runSingleGame started" << std::endl;
    
    GameExecution execution;
    execution.gameManagerName = gameManagerName;
    execution.algorithmName = algorithm1Name; // Keep for backward compatibility
    execution.mapName = map.name;
    execution.player1Name = "Player1";
    execution.player2Name = "Player2";
    
    auto startTime = std::chrono::steady_clock::now();
    
    try {
        std::cout << "DEBUG: Creating GameManager..." << std::endl;
        // Create GameManager
        auto gameManager = gameManagerFactory(verbose);
        if (!gameManager) {
            throw std::runtime_error("Failed to create GameManager");
        }
        std::cout << "DEBUG: GameManager created successfully" << std::endl;
        
        std::cout << "DEBUG: Creating Players..." << std::endl;
        // Create Players (same factory creates both players with different indices)
        auto player1 = playerFactory(1, map.width, map.height, map.maxSteps, map.numShells);
        auto player2 = playerFactory(2, map.width, map.height, map.maxSteps, map.numShells);
        if (!player1 || !player2) {
            throw std::runtime_error("Failed to create Players");
        }
        std::cout << "DEBUG: Players created successfully" << std::endl;
        
        std::cout << "DEBUG: Creating SatelliteView..." << std::endl;
        // Create SatelliteView from map
        auto satelliteView = createSatelliteView(map);
        if (!satelliteView) {
            throw std::runtime_error("Failed to create SatelliteView");
        }
        std::cout << "DEBUG: SatelliteView created successfully" << std::endl;
        
        std::cout << "DEBUG: About to call gameManager->run()..." << std::endl;
        // Run the game
        execution.result = gameManager->run(
            map.width, map.height,
            *satelliteView,
            map.name,
            map.maxSteps, map.numShells,
            *player1, algorithm1Name,  // Use first algorithm name
            *player2, algorithm2Name,  // Use second algorithm name
            tankAlgorithmFactory,
            tankAlgorithmFactory
        );
        std::cout << "DEBUG: gameManager->run() completed successfully!" << std::endl;
        
        execution.success = true;
        
    } catch (const std::exception& e) {
        execution.success = false;
        execution.errorMessage = e.what();
        std::cerr << "Game execution failed: " << e.what() << std::endl;
    } catch (...) {
        execution.success = false;
        execution.errorMessage = "Unknown error occurred";
        std::cerr << "Game execution failed with unknown error" << std::endl;
    }
    
    auto endTime = std::chrono::steady_clock::now();
    execution.executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
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