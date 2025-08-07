#ifndef GAMEMANAGER_318772340_206580102_H
#define GAMEMANAGER_318772340_206580102_H

#include <memory>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <string>

#include "../common/AbstractGameManager.h"
#include "../common/GameResult.h"
#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "../common/SatelliteView.h"
#include "Board.h"
#include "Tank.h"
#include "MySatelliteView.h"

#include "../UserCommon/Position.h"
#include "../UserCommon/Direction.h"
#include "../common/ActionRequest.h"

// Use UserCommon namespace
using namespace UserCommon_318772340_206580102;

namespace GameManager_318772340_206580102 {

using std::string;

class GameManager_318772340_206580102 : public AbstractGameManager {
private:
    bool verbose;
    std::unique_ptr<Board> gameBoard;
    std::unique_ptr<Player> player1_ptr;
    std::unique_ptr<Player> player2_ptr;
    size_t currentStep = 0;
    std::vector<Tank*> tanks1;
    std::vector<Tank*> tanks2;
    int stepsWithoutShells = 0;
    std::vector<std::pair<std::unique_ptr<TankAlgorithm>, Tank*>> tankPairs;
    std::ofstream simpleOutput; // For game result logging
    std::vector<Tank*> tanksOrderedByBirth;
    std::vector<std::string> currentActions;
    
    struct TankLogInfo {
        char symbol; // '1' או '2'
        bool isAlive = true;
        bool wasKilledThisTurn = false;
        std::string lastAction = "DoNothing";
    };
    std::vector<TankLogInfo> tankLog;
    
    // Game configuration
    size_t maxSteps = 0;
    size_t numShells = 0;
    const int NO_SHELL_LIMIT = 40;
    int tankIndex1 = 0;
    int tankIndex2 = 0;

public:
    GameManager_318772340_206580102(bool verbose);
    virtual ~GameManager_318772340_206580102();
    GameManager_318772340_206580102(const GameManager_318772340_206580102&) = delete;
    GameManager_318772340_206580102& operator=(const GameManager_318772340_206580102&) = delete;
    GameManager_318772340_206580102(GameManager_318772340_206580102&&) = delete;
    GameManager_318772340_206580102& operator=(GameManager_318772340_206580102&&) = delete;
    
    // Inherit from AbstractGameManager  
    virtual GameResult run(
        size_t map_width, size_t map_height,
        const SatelliteView& map, // <= a snapshot, NOT updated
        string map_name,
        size_t max_steps, size_t num_shells,
        Player& player1, string name1, Player& player2, string name2, 
        TankAlgorithmFactory player1_tank_algo_factory,
        TankAlgorithmFactory player2_tank_algo_factory) override;

    // Additional public methods from original GameManager
    Board& getBoard() { return *gameBoard; }

private:
    bool handleTankAction(TankAlgorithm& algo, Player& player, Tank* tank,
                          MySatelliteView& satellite,
                          std::unordered_map<TankAlgorithm*, Tank*>& tankMap,
                          std::unordered_map<TankAlgorithm*, Tank*>::iterator& it);
    
    void parseMapFromSatelliteView(const SatelliteView& map, size_t width, size_t height, 
                                  Player& player1, Player& player2,
                                  TankAlgorithmFactory factory1, TankAlgorithmFactory factory2);
    GameResult executeGameLoop(Player& player1, Player& player2);
    void executeAction(const ActionRequest& req, Tank* tank);
    void checkCollisions();
    void checkTankTankCollisions();
    void checkTankMineCollisions();
    void checkShellCollisions();
    bool checkGameOver() const;
    bool noShellsLeftForAllLiveTanks() const;
    void moveShells();
    void wrapPosition(Position& pos);
    std::vector<std::vector<char>> buildBoardMatrix();
    void populateTankOrderAndLog(size_t rows, size_t cols);
    void logGameResult();
    bool handleBackwardStatus(Tank* tank, const ActionRequest& req, const Position& pos, const Direction& dir);

};

} // namespace GameManager_318772340_206580102

#endif // GAMEMANAGER_318772340_206580102_H