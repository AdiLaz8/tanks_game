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
#include "MyTankAlgorithm.h"
#include "SatelliteView.h"

class GameManager {
private:
    std::ofstream logFile;
    std::unique_ptr<Board> gameBoard;
    std::unique_ptr<Player> player1;
    std::unique_ptr<Player> player2;
    std::unordered_map<MyTankAlgorithm*, Tank*> tankMap1;
    std::unordered_map<MyTankAlgorithm*, Tank*> tankMap2;

    std::string inputFileName;
    int numShells = 0;
    int maxSteps = 0;
    int currentStep = 0;

public:
    GameManager(std::string inputFileName);

    ~GameManager();

    void readBoard(const std::string& filename);
    void gameLoop();

private:
    bool handleTankAction(MyTankAlgorithm& algo, Player& player, Tank* tank,
                          SatelliteView& satellite,
                          std::unordered_map<MyTankAlgorithm*, Tank*>& tankMap,
                          std::unordered_map<MyTankAlgorithm*, Tank*>::iterator& it);

    void executeAction(const ActionRequest& req, MyTankAlgorithm& algo, Tank* tank);

    void moveShells();
    void checkCollisions();

    bool checkGameOver() const;
    void logGameResult();
    void wrapPosition(Position& pos);
    std::vector<std::vector<char>> buildBoardMatrix();
};

#endif // GAMEMANAGER_H
