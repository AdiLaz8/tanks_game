#include "GameManager.h"
#include "Shell.h"
#include "Algorithm1.h"
#include "Algorithm2.h"

GameManager::GameManager(Board& board) : gameBoard(board), currentStep(0), isAlgo1Turn(true), postAmmoSteps(80) {
    tank1 = gameBoard.getTank(1);
    tank2 = gameBoard.getTank(2);
    algorithm1 = std::make_unique<Algorithm1>();
    algorithm2 = std::make_unique<Algorithm2>();
}

void GameManager::gameLoop() {
    while (!checkGameOver()) {
        if (tank1->getAmmo() == 0 && tank2->getAmmo() == 0){
            postAmmoSteps == postAmmoSteps - 1;
        }
        if (currentStep % 2 != 0) {
            moveShells();
            checkCollisions();
        } else {
            Tank* currentTank = isAlgo1Turn ? tank1 : tank2;
            Tank* enemyTank = isAlgo1Turn ? tank2 : tank1;
            IAlgorithm& currentAlgo = isAlgo1Turn ? *algorithm1 : *algorithm2;
            moveShells();
            checkCollisions();
            if (checkGameOver())
            {
                break;
            }
            executeTankAction(currentTank, enemyTank, currentAlgo);
            checkCollisions();
            isAlgo1Turn = !isAlgo1Turn; // Toggle turn
        }
        currentStep++;
    }
}

void GameManager::moveShells() {
    auto& shells = gameBoard.getShells();
    for (auto it = shells.begin(); it != shells.end();) {
        Shell* shell = *it;
        shell->move(gameBoard.getWidth(), gameBoard.getHeight());
    }
}

void GameManager::executeTankAction(Tank* tank, Tank* enemyTank, IAlgorithm& algo) {
    if (!tank) return;
    Action action = algo.nextAction(gameBoard, *tank, *enemyTank);

    // Manage backward status outside of switch for clarity and control
    if (tank->getBackwardStatus() > 2) {
        tank->decreaseBackward();
        return; // Skip action execution if in the middle of backward delay
    } else if (tank->getBackwardStatus() == 2) {
        tank->moveBackward(gameBoard.getWidth(), gameBoard.getHeight());
        tank->decreaseBackward();
        return;
    } else if (tank->getBackwardStatus() == 1) {
        if (action.getType() != ActionType::MoveBackward) {
            tank->setBackward(0); // Reset if not continuing to move backward
        }
        else {
            tank->moveBackward(gameBoard.getWidth(), gameBoard.getHeight());
            tank->decreaseBackward();
            return;
        }
    }

    switch (action.getType()) {
        case ActionType::MoveForward:
            if (tank->getBackwardStatus() == 0) {
                tank->moveForward(gameBoard.getWidth(), gameBoard.getHeight());
            }
            else {
                tank->setBackward(0);
                return;
            }
            break;
        case ActionType::MoveBackward:
            if (tank->getBackwardStatus() == 0) {
                tank->setBackward(4); // Initialize backward movement
            }
            break;
        case ActionType::Shoot:
            if (tank->getShootingStatus() == 0 && tank->getAmmo() > 0) {
                tank->shoot();
                Shell* newShell = new Shell(tank->getPosition(), tank->getDirection(), tank->getSymbol());
                gameBoard.addObject(newShell, newShell->getPosition().x, newShell->getPosition().y);
            }
            break;
        case ActionType::RotateLeft8:
            tank->rotateLeft8();
            break;
        case ActionType::RotateRight8:
            tank->rotateRight8();
            break;
        case ActionType::RotateLeft4:
            tank->rotateLeft4();
            break;
        case ActionType::RotateRight4:
            tank->rotateRight4();
            break;
        default:
            break;
    }

    // Always attempt to decrease shooting status if it's above 0
    if (tank->getShootingStatus() > 0) {
        tank->decreaseShooting();
    }
}

void GameManager::checkCollisions() {
    auto& shells = gameBoard.getShells();
    Position posTank1 = tank1->getPosition();
    Position posTank2 = tank2->getPosition();
    
    for (Shell* shell : shells) {
        Position shellPos = shell->getPosition();
        bool hitTank1 = (shellPos == posTank1);
        bool hitTank2 = (shellPos == posTank2);

        if (hitTank1 && hitTank2) {
            // Both tanks are hit, it's a tie
            tank1->hit();
            tank2->hit();
            gameBoard.removeObject(tank1, posTank1.x, posTank1.y);
            gameBoard.removeObject(tank2, posTank2.x, posTank2.y);
            break; // End game on a tie
        } else if (hitTank1) {
            // Only Tank1 is hit
            tank1->hit();
            gameBoard.removeObject(tank1, posTank1.x, posTank1.y);
        } else if (hitTank2) {
            // Only Tank2 is hit
            tank2->hit();
            gameBoard.removeObject(tank2, posTank2.x, posTank2.y);
        }    
        // Remove the shell if it hits any tank
        if (hitTank1 || hitTank2) {
            gameBoard.removeObject(shell, shellPos.x, shellPos.y);
        }
    }
}


bool GameManager::checkGameOver() {
    if (tank1->getAmmo() == 0 && tank2->getAmmo() == 0 && postAmmoSteps-- <= 0)
        return true;
    if (!tank1->isAlive() || !tank2->isAlive())
        return true;
    return false;
}