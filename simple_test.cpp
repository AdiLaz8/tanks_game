#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "../common/AbstractGameManager.h"
#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "../common/SatelliteView.h"
#include "../common/GameResult.h"
#include "../common/BattleInfo.h"

// Simple SatelliteView implementation for testing
class TestSatelliteView : public SatelliteView {
private:
    std::vector<std::vector<char>> mapData;
    size_t width, height;
    
public:
    TestSatelliteView(const std::vector<std::vector<char>>& data) 
        : mapData(data), width(data.empty() ? 0 : data[0].size()), height(data.size()) {}
    
    char getObjectAt(size_t x, size_t y) const override {
        if (y >= height || x >= width) {
            return '&'; // Out of bounds
        }
        return mapData[y][x];
    }
};

// Simple TankAlgorithm implementation for testing
class TestTankAlgorithm : public TankAlgorithm {
public:
    ActionRequest getAction() override {
        return ActionRequest::DoNothing;
    }
    
    void updateBattleInfo(BattleInfo& /*info*/) override {
        // Do nothing for testing
    }
};

// Simple Player implementation for testing
class TestPlayer : public Player {
public:
    TestPlayer(int /*id*/) {}
    
    void updateTankWithBattleInfo(TankAlgorithm& /*tank*/, SatelliteView& /*satellite_view*/) override {
        // Do nothing for testing
    }
};

// Simple GameManager implementation for testing
class TestGameManager : public AbstractGameManager {
public:
    TestGameManager(bool /*verbose*/) {}
    
    GameResult run(
        size_t map_width, size_t map_height,
        const SatelliteView& map,
        std::string map_name,
        size_t max_steps, size_t num_shells,
        Player& /*player1*/, std::string name1, Player& /*player2*/, std::string name2,
        TankAlgorithmFactory /*player1_tank_algo_factory*/,
        TankAlgorithmFactory /*player2_tank_algo_factory*/) override {
        
        std::cout << "=== TestGameManager::run called ===" << std::endl;
        std::cout << "Map: " << map_name << std::endl;
        std::cout << "Dimensions: " << map_width << "x" << map_height << std::endl;
        std::cout << "MaxSteps: " << max_steps << std::endl;
        std::cout << "NumShells: " << num_shells << std::endl;
        std::cout << "Player1: " << name1 << std::endl;
        std::cout << "Player2: " << name2 << std::endl;
        
        // Print the map
        std::cout << "\nMap Layout:" << std::endl;
        for (size_t y = 0; y < map_height; ++y) {
            std::cout << "Row " << y << ": ";
            for (size_t x = 0; x < map_width; ++x) {
                std::cout << map.getObjectAt(x, y);
            }
            std::cout << std::endl;
        }
        
        // Create a simple result
        GameResult result;
        result.winner = 1;
        result.reason = GameResult::Reason::MAX_STEPS;
        result.rounds = 10;
        result.remaining_tanks = {1, 0}; // Player 1 has 1 tank, Player 2 has 0
        
        std::cout << "\n=== TestGameManager::run completed ===" << std::endl;
        return result;
    }
    
};

int main() {
    std::cout << "=== Simple GameManager Test ===" << std::endl;
    
    // Create a simple test map (similar to map1.txt)
    std::vector<std::vector<char>> testMap = {
        {' ', ' ', ' ', '1', ' ', ' ', ' ', ' ', '#', ' ', ' ', '@', ' ', ' ', ' ', ' ', ' '},
        {' ', '#', ' ', ' ', ' ', '@', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' '},
        {' ', '@', ' ', ' ', ' ', '#', ' ', ' ', ' ', '@', ' ', ' ', ' ', '#', ' ', ' ', ' '},
        {'1', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', '2', ' ', ' '},
        {' ', '#', ' ', ' ', ' ', ' ', ' ', '@', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', '@', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' '},
        {'2', ' ', ' ', ' ', '#', ' ', ' ', ' ', '@', ' ', ' ', ' ', ' ', ' ', '1', ' ', ' '},
        {' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '@', ' ', ' ', ' ', '#', ' '},
        {' ', ' ', ' ', '1', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', '@', ' '},
        {' ', '#', ' ', ' ', ' ', '@', ' ', ' ', ' ', ' ', ' ', '2', ' ', ' ', ' ', '#', ' '},
        {' ', ' ', ' ', '@', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '1', ' ', ' ', ' '},
        {'2', ' ', ' ', ' ', '#', ' ', ' ', ' ', '@', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' '}
    };
    
    // Create SatelliteView
    auto satelliteView = std::make_unique<TestSatelliteView>(testMap);
    
    // Create players
    auto player1 = std::make_unique<TestPlayer>(1);
    auto player2 = std::make_unique<TestPlayer>(2);
    
    // Create GameManager
    auto gameManager = std::make_unique<TestGameManager>(true);
    
    // Create simple algorithm factories
    TankAlgorithmFactory simpleFactory = [](int /*player_index*/, int /*tank_index*/) {
        return std::make_unique<TestTankAlgorithm>();
    };
    
    // Run the game
    std::cout << "\n=== Running Game ===" << std::endl;
    GameResult result = gameManager->run(
        18, 12,  // map dimensions
        *satelliteView,
        "test_map1.txt",
        250, 6,  // maxSteps, numShells
        *player1, "Player1", *player2, "Player2",
        simpleFactory, simpleFactory
    );
    
    std::cout << "\n=== Game Result ===" << std::endl;
    std::cout << "Winner: " << result.winner << std::endl;
    std::cout << "Reason: " << static_cast<int>(result.reason) << std::endl;
    std::cout << "Rounds: " << result.rounds << std::endl;
    std::cout << "Remaining tanks: ";
    for (size_t i = 0; i < result.remaining_tanks.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << "Player" << (i+1) << "=" << result.remaining_tanks[i];
    }
    std::cout << std::endl;
    
    std::cout << "\n=== Test completed successfully! ===" << std::endl;
    return 0;
} 