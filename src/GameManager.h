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
    int tankMap1=0;
    int tankMap2=0;
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
    void readBoard(const std::string& filename);
    void gameLoop();

private:
    bool handleTankAction(TankAlgorithm& algo, Player& player, Tank* tank,
                          MySatelliteView& satellite,
                          std::unordered_map<TankAlgorithm*, Tank*>& tankMap,
                          std::unordered_map<TankAlgorithm*, Tank*>::iterator& it);

    void executeAction(const ActionRequest& req, Tank* tank);
    bool noShellsLeftForAllLiveTanks() const;
    void moveShells();
    void checkCollisions();
    void readBoardHeader(const std::string& filename, std::ifstream& file, size_t& rows, size_t& cols);
    bool checkGameOver() const;
    void logGameResult();
    void wrapPosition(Position& pos);
    std::vector<std::vector<char>> buildBoardMatrix();
    void populateTankOrderAndLog(size_t rows, size_t cols);
    void finalizeBoardReading(std::ifstream& file, std::ofstream& errorFile, bool hasTank1, bool hasTank2);
};

#endif // GAMEMANAGER_H
