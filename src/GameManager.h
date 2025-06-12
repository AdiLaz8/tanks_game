#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <memory>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <string>

#include "Board.h"
#include "Tank.h"
#include "Player.h"
#include "SatelliteView.h"
#include "PlayerFactory.h"
#include "TankAlgorithmFactory.h"
#include "MySatelliteView.h"
#include "TankAlgorithm.h"

class GameManager {
private:
    std::ofstream logFile;
    std::ifstream file;
    std::unique_ptr<Board> gameBoard;
    std::unique_ptr<Player> player1;
    std::unique_ptr<Player> player2;
    int tankIndex1 = 0;
    int tankIndex2 = 0;
    const int NO_SHELL_LIMIT = 40;
    size_t numShells = 0;
    size_t maxSteps = 0;
    const PlayerFactory& playerFactory;
    const TankAlgorithmFactory& tankAlgoFactory;
    size_t currentStep = 0;
    std::vector<Tank*> tanks1;
    std::vector<Tank*> tanks2;
    int stepsWithoutShells = 0;
    std::vector<std::pair<std::unique_ptr<TankAlgorithm>, Tank*>> tankPairs;
    std::ofstream simpleOutput;
    std::vector<Tank*> tanksOrderedByBirth;
    std::vector<std::string> currentActions;
    struct TankLogInfo {
        char symbol; // '1' או '2'
        bool isAlive = true;
        bool wasKilledThisTurn = false;
        std::string lastAction = "DoNothing";
    };
    std::vector<TankLogInfo> tankLog;

public:
    GameManager(const PlayerFactory& pf, const TankAlgorithmFactory& tf);
    ~GameManager();
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    GameManager(GameManager&&) = delete;
    GameManager& operator=(GameManager&&) = delete;
    void readBoard(const std::string& filename);
    Board& getBoard() { return *gameBoard; }
    void gameLoop();
    void executeAction(const ActionRequest& req, Tank* tank);
    void checkCollisions();
    bool checkGameOver() const;




private:
    bool handleTankAction(TankAlgorithm& algo, Player& player, Tank* tank,
                          MySatelliteView& satellite,
                          std::unordered_map<TankAlgorithm*, Tank*>& tankMap,
                          std::unordered_map<TankAlgorithm*, Tank*>::iterator& it);

    bool noShellsLeftForAllLiveTanks() const;
    void moveShells();
    void checkTankTankCollisions();
    void checkTankMineCollisions();
    void checkShellCollisions();
    void readBoardHeader(const std::string& filename, std::ifstream& file, size_t& rows, size_t& cols);
    void logGameResult();
    void wrapPosition(Position& pos);
    
    std::vector<std::vector<char>> buildBoardMatrix();
    void populateTankOrderAndLog(size_t rows, size_t cols);
    void finalizeBoardReading(std::ifstream& file, std::ofstream& errorFile, int& tankIndex1, int& tankIndex2);
    bool handleBackwardStatus(Tank* tank, const ActionRequest& req, const Position& pos, const Direction& dir, const std::string& player);
};

#endif // GAMEMANAGER_H