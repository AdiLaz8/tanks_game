#include "GameRunner.h"
#include <iostream>

// Forward declare MapSatelliteView to avoid including GameManager files
class MapSatelliteView;

// Simple function to print MapData contents
void printMapData(const MapData& map) {
    std::cout << "\n=== Map Data ===" << std::endl;
    std::cout << "Name: " << map.name << std::endl;
    std::cout << "File: " << map.filename << std::endl;
    std::cout << "Dimensions: " << map.width << "x" << map.height << std::endl;
    std::cout << "MaxSteps: " << map.maxSteps << std::endl;
    std::cout << "NumShells: " << map.numShells << std::endl;
    std::cout << "\nMap Layout:" << std::endl;
    
    for (size_t y = 0; y < map.height; ++y) {
        std::cout << "Row " << y << ": ";
        for (size_t x = 0; x < map.width; ++x) {
            char c = map.data[y][x];
            if (c == ' ') {
                std::cout << '.';  // Make spaces visible as dots
            } else {
                std::cout << c;
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "Testing Map Parsing..." << std::endl;
    
    // Test parsing different maps
    std::vector<std::string> mapFiles = {
        "test_map_5x5_3x3.txt",
        "test_map_5x5_5x4.txt", 
        "test_map_5x5_4x5.txt",
        "test_map_5x5_4x4.txt",
        "test_map_5x5_6x5.txt",
        "test_map_5x5_5x6.txt",
        "test_map_5x5_6x6.txt"
    };
    
    for (const auto& mapFile : mapFiles) {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "Testing: " << mapFile << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        MapData map;
        if (map.loadFromFile(mapFile)) {
            printMapData(map);
            
            // Test SatelliteView creation
            try {
                auto satelliteView = GameRunner::createSatelliteView(map);
                if (satelliteView) {
                    std::cout << "✓ SatelliteView created successfully!" << std::endl;
                } else {
                    std::cout << "✗ Failed to create SatelliteView (returned null)" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "✗ Exception creating SatelliteView: " << e.what() << std::endl;
            }
        } else {
            std::cout << "✗ Failed to load map file!" << std::endl;
        }
    }
    
    return 0;
}