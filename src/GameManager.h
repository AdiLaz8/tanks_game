#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "Board.h"
#include "IAlgorithm.h"
#include <memory>
#include <fstream>


class GameManager {
private:
    std::ofstream logFile;
    std::unique_ptr<Board> gameBoard;
    std::unique_ptr<IAlgorithm> algorithm1;
    std::unique_ptr<IAlgorithm> algorithm2;
    std::vector<Tank*> tanks1; // the tanks for player 1
    std::vector<Tank*> tanks2; // the tanks for player 2
    int currentStep; // a counter for the current step i the game, to know if to only move shells or to get actions for tanks as well
    int postAmmoSteps; // Steps after both tanks run out of ammo
    std::string inputFileName;
    int numShells;
    int maxSteps;

public:
    GameManager(std::string inputFileName);
    ~GameManager();
    void gameLoop();
    void readBoard(const std::string& filename);
    void moveShells();
    void executeTankAction(Tank* tank, Tank* enemyTank, IAlgorithm& algo);
    void checkCollisions();
    bool checkGameOver();
    const std::vector<Tank*>& getTanks1() const { return tanks1; }
    const std::vector<Tank*>& getTanks2() const { return tanks2; }
    Board& getBoard() const { return *gameBoard; }

};

#endif