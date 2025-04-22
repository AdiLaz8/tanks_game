#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "Board.h"
#include "IAlgorithm.h"
#include <memory>
#include <fstream>


class GameManager {
private:
    std::ofstream logFile;
    Board& gameBoard;
    std::unique_ptr<IAlgorithm> algorithm1;
    std::unique_ptr<IAlgorithm> algorithm2;
    Tank* tank1;
    Tank* tank2;
    int currentStep;
    int postAmmoSteps; // Steps after both tanks run out of ammo
    std::string inputFileName;

public:
    GameManager(Board& board,std::string inputFileName);
    ~GameManager();
    void gameLoop();
    void moveShells();
    void executeTankAction(Tank* tank, Tank* enemyTank, IAlgorithm& algo);
    void checkCollisions();
    bool checkGameOver();
};

#endif // GAMEMANAGER_H
