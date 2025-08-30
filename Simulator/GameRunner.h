#ifndef SIMULATOR_GAME_RUNNER_H
#define SIMULATOR_GAME_RUNNER_H

#include <string>
#include <memory>
#include <chrono>
#include "../common/AbstractGameManager.h"
#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "../common/SatelliteView.h"
#include "../common/GameResult.h"
#include "SimulationResults.h"

struct MapData {
    std::string name;
    std::string filename;
    size_t width;
    size_t height;
    size_t maxSteps;
    size_t numShells;
    std::vector<std::vector<char>> data;
    
    bool loadFromFile(const std::string& filepath, bool verbose = false);
};

class GameRunner {
public:
    static GameExecution runSingleGame(
        GameManagerFactory gameManagerFactory,
        const std::string& gameManagerName,
        PlayerFactory playerFactory,
        TankAlgorithmFactory tankAlgorithmFactory,
        const std::string& algorithm1Name,
        const std::string& algorithm2Name,
        const MapData& map,
        bool verbose = false
    );
    
    static std::unique_ptr<class MapSatelliteView> createSatelliteView(const MapData& map);
    
private:
};

class MapSatelliteView : public SatelliteView {
private:
    std::vector<std::vector<char>> mapData;
    size_t width, height;
    
public:
    MapSatelliteView(const std::vector<std::vector<char>>& data);
    char getObjectAt(size_t x, size_t y) const override;
    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }
};

#endif // SIMULATOR_GAME_RUNNER_H