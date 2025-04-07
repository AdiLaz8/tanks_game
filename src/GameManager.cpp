#include "GameManager.h"
#include "Shell.h"
#include "Algorithm1.h"
#include "Algorithm2.h"

GameManager::GameManager(Board& board) : gameBoard(board), currentStep(0), isAlgo1Turn(true), postAmmoSteps(80) {
    tank1 = gameBoard.getTank(1);
    tank2 = gameBoard.getTank(2);
    algorithm1 = std::make_unique<Algorithm1>();
    algorithm2 = std::make_unique<Algorithm2>();
    logFile.open("game_output.txt"); 

}
GameManager::~GameManager() {
    if (logFile.is_open()) {
        logFile.close();
    }
}


void GameManager::gameLoop() {
    while (!checkGameOver()) {
        if (tank1->getAmmo() == 0 && tank2->getAmmo() == 0){
            postAmmoSteps = postAmmoSteps - 1;
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
    logFile << "Game Over!" << std::endl;
    if (!tank1->isAlive() && !tank2->isAlive()) {
        logFile << "Result: Tie - Both tanks destroyed" << std::endl;
    } else if (!tank1->isAlive()) {
        logFile << "Result: Player 2 wins - Player 1 destroyed" << std::endl;
    } else if (!tank2->isAlive()) {
        logFile << "Result: Player 1 wins - Player 2 destroyed" << std::endl;
    } else if (tank1->getAmmo() == 0 && tank2->getAmmo() == 0 && postAmmoSteps <= 0) {
        logFile << "Result: Tie - No ammo left and time ended" << std::endl;
    } else {
        logFile << "Result: Unknown" << std::endl;
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
    std::string player = (tank->getSymbol() == '1') ? "Player 1" : "Player 2";
    //bool badStep = false;
    // Manage backward status outside of switch for clarity and control
    if (tank->getBackwardStatus() > 2) {
        tank->decreaseBackward();
        logFile << player << ": Nothing happened, part of MoveBackward waiting" << std::endl;
        return; // Skip action execution if in the middle of backward delay
    }
    else if (tank->getBackwardStatus() == 2) {
        tank->moveBackward(gameBoard.getWidth(), gameBoard.getHeight());
        tank->decreaseBackward();
        logFile << player << ": MoveBackward" << std::endl;
        return;
    }
    else if (tank->getBackwardStatus() == 1) {

        if (action.getType() != ActionType::MoveBackward) {
            tank->setBackward(0); // Reset if not continuing to move backward
        }
        else {
            Position next = tank->getPosition() + tank->getDirection().getOppositeDirection().toVector();
            if(gameBoard.isPassable(next.x,next.y)) {
                tank->moveBackward(gameBoard.getWidth(), gameBoard.getHeight());
                logFile << player << ": MoveBackward" << std::endl;
            }
            else{
                logFile << player << ": MoveBackward failed-wall (Bad Step)" << std::endl;
                tank->decreaseBackward();
                //badStep = true;
            }
            return;
        }
    }
    Position next(0,0);
    switch (action.getType()) {
        case ActionType::MoveForward:
            next=tank->getPosition() + tank->getDirection().toVector();
            if (tank->getBackwardStatus() == 0){
                if(gameBoard.isPassable(next.x, next.y)) {
                    tank->moveForward(gameBoard.getWidth(), gameBoard.getHeight());
                    logFile << player << ": MoveForward" << std::endl;
                }
                else{
                    logFile << player << ": MoveForward failed-wall (Bad Step)" << std::endl;
                    //badStep = true;
                }
            }
            else {
                tank->setBackward(0);
                logFile << player << ": Canceled moving BackWards" << std::endl;
                return;
            }
            break;
        case ActionType::MoveBackward:
            if (tank->getBackwardStatus() == 0) {
                next = tank->getPosition() + tank->getDirection().getOppositeDirection().toVector();
                if(gameBoard.isPassable(next.x,next.y)) {
                    tank->setBackward(4); // Initialize backward movement
                    logFile << player << ": Started MoveBackward proccess" << std::endl;
                }
                else{
                    logFile << player << ": MoveBackward failed-wall (Bad Step)" << std::endl;
                    //badStep = true;
                }
            }
            break;
        case ActionType::Shoot:
            if (tank->getShootingStatus() == 0 && tank->getAmmo() > 0) {
                tank->shoot();
                Position pos = tank->getPosition() + tank->getDirection().toVector();
                int x = (pos.x + gameBoard.getWidth()) % gameBoard.getWidth();
                int y = (pos.y + gameBoard.getHeight()) % gameBoard.getHeight();
                Shell* newShell = new Shell(Position(x, y), tank->getDirection(), tank->getSymbol());
                gameBoard.addObject(newShell, x, y);
                logFile << player << ": Shoot" << std::endl;
            }
            else{
                logFile << player << ": Shoot (Bad Step)" << std::endl;
                //badStep = true;
            }
            break;
        case ActionType::RotateLeft8:
            tank->rotateLeft8();
            logFile << player << ": RotateLeft8" << std::endl;

            break;
        case ActionType::RotateRight8:
            tank->rotateRight8();
            logFile << player << ": RotateRight8" << std::endl;
            break;
        case ActionType::RotateLeft4:
            tank->rotateLeft4();
            logFile << player << ": RotateLeft4" << std::endl;
            break;
        case ActionType::RotateRight4:
            tank->rotateRight4();
            logFile << player << ": RotateRight4" << std::endl;
            break;
        default:
            logFile << player << ": None" << std::endl;
            break;
    }


}

void GameManager::checkCollisions() {
    auto& shells = gameBoard.getShells();
    Position posTank1 = tank1->getPosition();
    Position posTank2 = tank2->getPosition();
    if (posTank1 == posTank2 && tank1->isAlive() && tank2->isAlive()) {
        tank1->hit();
        tank2->hit();
        gameBoard.removeObject(tank1, posTank1.x, posTank1.y);
        gameBoard.removeObject(tank2, posTank2.x, posTank2.y);
        logFile << "Collision: Tank1 and Tank2 collided -> Tie (both destroyed)" << std::endl;
        return;
    }
    for (Tank* tank : {tank1, tank2}) {
        Position pos = tank->getPosition();
        CellSlot& slot = gameBoard.getSlot(pos.x, pos.y);
        if (slot.getMine()) {
            gameBoard.removeObject(tank, pos.x, pos.y);
            tank->hit();
            std::string player = (tank == tank1 ? "Player 1" : "Player 2");
            logFile << "Mine: " << player << " hit a mine and was destroyed. Enemy wins." << std::endl;
            return;
        }
    }
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
            logFile << "Shell: Player 1 was hit by shell and destroyed." << std::endl;
            logFile << "Shell: Player 2 was hit by shell and destroyed." << std::endl;
            break; // End game on a tie
        } else if (hitTank1) {
            // Only Tank1 is hit
            tank1->hit();
            gameBoard.removeObject(tank1, posTank1.x, posTank1.y);
            logFile << "Shell: Player 1 was hit by shell and destroyed." << std::endl;
        } else if (hitTank2) {
            // Only Tank2 is hit
            tank2->hit();
            gameBoard.removeObject(tank2, posTank2.x, posTank2.y);
            logFile << "Shell: Player 2 was hit by shell and destroyed." << std::endl;
        }
        // Remove the shell if it hits any tank
        if (hitTank1 || hitTank2) {
            gameBoard.removeObject(shell, shellPos.x, shellPos.y);
        }
        CellSlot& slot = gameBoard.getSlot(shellPos.x, shellPos.y);
        if (slot.getWall()) {
            int hp = slot.getWall()->onHit();
            logFile << "Shell hit wall. Wall health is now " << hp << std::endl;
            if (hp <= 0) {
                gameBoard.removeObject(shell, shellPos.x, shellPos.y);
                logFile << "Wall destroyed." << std::endl;
            }
            continue;
        }

        // פגיעת פגז בפגז אחר
        for (Shell* otherShell : slot.getShells()) {
            Position shellPos2 = otherShell->getPosition();
            if (otherShell != shell && shellPos==shellPos2 ) {
                gameBoard.removeObject(shell, shellPos.x, shellPos.y);
                gameBoard.removeObject(otherShell, shellPos2.x, shellPos2.y);
                logFile << "Shell: Two shells collided and were destroyed." << std::endl;
                break;
            }
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