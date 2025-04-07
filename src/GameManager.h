#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "Board.h"
#include "IAlgorithm.h"
#include <memory>

class GameManager {
private:
    Board& gameBoard;
    std::unique_ptr<IAlgorithm> algorithm1;
    std::unique_ptr<IAlgorithm> algorithm2;
    Tank* tank1;
    Tank* tank2;
    int currentStep;
    bool isAlgo1Turn;
    int postAmmoSteps; // Steps after both tanks run out of ammo

public:
    GameManager(Board& board);
    ~GameManager() = default;
    void gameLoop();
    void moveShells();
    void executeTankAction(Tank* tank, Tank* enemyTank, IAlgorithm& algo);
    void checkCollisions();
    bool checkGameOver();
};

#endif // GAMEMANAGER_H
