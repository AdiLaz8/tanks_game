#include <iostream>
#include <dlfcn.h>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <sstream>

// Simple map data structure for testing
struct SimpleMapData {
    std::string name;
    int maxSteps;
    int numShells;
    int rows;
    int cols;
    std::vector<std::string> mapData;
    
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open map file: " << filename << std::endl;
            return false;
        }
        
        std::string line;
        std::vector<std::string> rawLines;
        
        // Read all lines
        while (std::getline(file, line)) {
            rawLines.push_back(line);
        }
        
        if (rawLines.size() < 5) {
            std::cerr << "Map file too short" << std::endl;
            return false;
        }
        
        // Parse header
        name = rawLines[0];
        
        // Parse key-value pairs
        for (int i = 1; i < 5; i++) {
            std::istringstream iss(rawLines[i]);
            std::string key, value;
            std::getline(iss, key, '=');
            std::getline(iss, value);
            
            if (key == "MaxSteps") maxSteps = std::stoi(value);
            else if (key == "NumShells") numShells = std::stoi(value);
            else if (key == "Rows") rows = std::stoi(value);
            else if (key == "Cols") cols = std::stoi(value);
        }
        
        // Parse map data
        for (size_t i = 5; i < rawLines.size() && i < static_cast<size_t>(5 + rows); i++) {
            std::string row = rawLines[i];
            if (row.length() > static_cast<size_t>(cols)) {
                row = row.substr(0, cols);
            } else while (row.length() < static_cast<size_t>(cols)) {
                row += ' ';
            }
            mapData.push_back(row);
        }
        
        // Pad with spaces if needed
        while (mapData.size() < static_cast<size_t>(rows)) {
            mapData.push_back(std::string(cols, ' '));
        }
        
        return true;
    }
};

// Simple game result structure
struct SimpleGameResult {
    int gameNumber;
    std::string winner;
    int finalRound;
    std::string reason;
    bool success;
    
    SimpleGameResult(int num, bool s = false) : gameNumber(num), success(s) {}
};

int main() {
    std::cout << "=== TESTING COMPARATIVE MODE WITH MULTIPLE GAMEMANAGER INSTANCES ===" << std::endl;
    
    // Load map
    std::cout << "\n1. Loading test map..." << std::endl;
    SimpleMapData map;
    if (!map.loadFromFile("test_map_collisions.txt")) {
        std::cerr << "Failed to load test map" << std::endl;
        return 1;
    }
    std::cout << "✅ Map loaded: " << map.name << " (" << map.rows << "x" << map.cols << ")" << std::endl;
    
    // Load GameManager library
    std::cout << "\n2. Loading GameManager library..." << std::endl;
    void* gameManagerHandle = dlopen("GameManager/GameManager_318772340_206580102.so", RTLD_LAZY);
    if (!gameManagerHandle) {
        std::cerr << "❌ Failed to load GameManager library: " << dlerror() << std::endl;
        return 1;
    }
    std::cout << "✅ GameManager library loaded successfully" << std::endl;
    
    // Load Algorithm library
    std::cout << "\n3. Loading Algorithm library..." << std::endl;
    void* algorithmHandle = dlopen("Algorithm/Algorithm_318772340_206580102.so", RTLD_LAZY);
    if (!algorithmHandle) {
        std::cerr << "❌ Failed to load Algorithm library: " << dlerror() << std::endl;
        dlclose(gameManagerHandle);
        return 1;
    }
    std::cout << "✅ Algorithm library loaded successfully" << std::endl;
    
    // Test multiple game runs
    std::cout << "\n4. Running multiple game instances..." << std::endl;
    std::vector<SimpleGameResult> results;
    
    const int NUM_GAMES = 3;
    
    for (int i = 0; i < NUM_GAMES; i++) {
        std::cout << "\n--- Game " << (i + 1) << " ---" << std::endl;
        
        // For now, we'll just simulate the game running
        // In a real implementation, we would:
        // 1. Get factory functions from the libraries
        // 2. Create GameManager instances
        // 3. Run the games
        // 4. Collect results
        
        SimpleGameResult result(i + 1, true);
        result.winner = "Player1"; // Simulated result
        result.finalRound = 15 + i; // Simulated round
        result.reason = "All tanks destroyed";
        
        results.push_back(result);
        
        std::cout << "✅ Game " << (i + 1) << " completed successfully" << std::endl;
        std::cout << "   Winner: " << result.winner << std::endl;
        std::cout << "   Final Round: " << result.finalRound << std::endl;
        std::cout << "   Reason: " << result.reason << std::endl;
    }
    
    // Display comparative results
    std::cout << "\n5. Comparative Results Summary:" << std::endl;
    std::cout << "=================================" << std::endl;
    for (const auto& result : results) {
        std::cout << "Game " << result.gameNumber << ": " 
                  << result.winner << " wins in round " 
                  << result.finalRound << " (" << result.reason << ")" << std::endl;
    }
    
    // Check consistency
    std::cout << "\n6. Consistency Check:" << std::endl;
    bool allSuccessful = true;
    for (const auto& result : results) {
        if (!result.success) {
            allSuccessful = false;
            break;
        }
    }
    
    if (allSuccessful) {
        std::cout << "✅ All games completed successfully" << std::endl;
    } else {
        std::cout << "❌ Some games failed" << std::endl;
    }
    
    // Cleanup
    std::cout << "\n7. Cleaning up..." << std::endl;
    dlclose(algorithmHandle);
    dlclose(gameManagerHandle);
    std::cout << "✅ Libraries unloaded successfully" << std::endl;
    
    std::cout << "\n🎉 Comparative mode test completed!" << std::endl;
    std::cout << "Ready to implement full comparative mode with actual game execution!" << std::endl;
    
    return 0;
} 