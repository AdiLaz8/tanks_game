#include "GameManager.h"
#include "Shell.h"
#include "Algorithm1.h"
#include "Algorithm2.h"
#include <iostream>


GameManager::GameManager(Board& board) : gameBoard(board), currentStep(0), postAmmoSteps(80) {
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
            IAlgorithm& algo1 = *algorithm1;
            moveShells();
            checkCollisions();
            if (checkGameOver())
            {
                break;
            }
            executeTankAction(tank1, tank2, algo1);
            IAlgorithm& algo2 = *algorithm2;
            executeTankAction(tank2, tank1, algo2);
            checkCollisions();
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
    std::vector<Shell*> toMove = shells; // יוצרים עותק של כל הפגזים הקיימים

    for (Shell* shell : toMove) {
        Position oldPos = shell->getPosition();

        // הסרה מהמיקום הנוכחי בלוח
        gameBoard.removeObject(shell, oldPos.x, oldPos.y);

        // תזוזה לוגית של הפגז
        shell->move(gameBoard.getWidth(), gameBoard.getHeight());

        Position newPos = shell->getPosition();

        // החזרה ללוח במיקום החדש
        gameBoard.addObject(shell, newPos.x, newPos.y);

        std::cout << "📦 Shell moved from (" << oldPos.x << "," << oldPos.y << ") to (" << newPos.x << "," << newPos.y << ")" << std::endl;
    }
}


void GameManager::executeTankAction(Tank* tank, Tank* enemyTank, IAlgorithm& algo) {
    if (!tank) return;
    Action action = algo.nextAction(gameBoard, *tank, *enemyTank);
    std::string player = (tank->getSymbol() == '1') ? "Player 1" : "Player 2";
    Position initialPosition = tank->getPosition();
    Direction direction = tank->getDirection();

    // Logging to console for debugging
    std::cout << player << " initiates action: ";
    switch (action.getType()) {
        case ActionType::MoveForward: {
            Position newPosition = initialPosition + direction.toVector();
            if (gameBoard.isPassable(newPosition.x, newPosition.y)) {
                tank->moveForward(gameBoard.getWidth(), gameBoard.getHeight());
                logFile << player << ": MoveForward from (" << initialPosition.x << ", " << initialPosition.y << ") to (" << newPosition.x << ", " << newPosition.y << ")." << std::endl;
                std::cout << "Moved forward from (" << initialPosition.x << ", " << initialPosition.y << ") to (" << newPosition.x << ", " << newPosition.y << ").\n";
            } else {
                logFile << player << ": MoveForward failed due to obstruction." << std::endl;
                std::cout << "Failed to move forward due to obstruction.\n";
            }
            break;
        }
        case ActionType::MoveBackward: {
            if (tank->getBackwardStatus() == 0) {
                Position newPosition = initialPosition + direction.getOppositeDirection().toVector();
                if (gameBoard.isPassable(newPosition.x, newPosition.y)) {
                    tank->setBackward(4);
                    logFile << player << ": Started MoveBackward process." << std::endl;
                    std::cout << "Started MoveBackward process from (" << initialPosition.x << ", " << initialPosition.y << ") to (" << newPosition.x << ", " << newPosition.y << ").\n";
                } else {
                    logFile << player << ": MoveBackward failed due to obstruction." << std::endl;
                    std::cout << "Failed to move backward due to obstruction.\n";
                }
            }
            break;
        }
        case ActionType::Shoot: {
            if (tank->getShootingStatus() == 0 && tank->getAmmo() > 0) {
                Position shootPosition = initialPosition + direction.toVector();
                int x = (shootPosition.x + gameBoard.getWidth()) % gameBoard.getWidth();
                int y = (shootPosition.y + gameBoard.getHeight()) % gameBoard.getHeight();
                Shell* newShell = new Shell(Position(x, y), direction, tank->getSymbol());
                gameBoard.addObject(newShell, x, y);
                logFile << player << ": Shoot from (" << initialPosition.x << ", " << initialPosition.y << ") to (" << x << ", " << y << ") in direction " << direction.getDirection() << "." << std::endl;
                std::cout << "Shot from (" << initialPosition.x << ", " << initialPosition.y << ") to (" << x << ", " << y << ") in direction " << direction.getDirection() << ".\n";
            } else {
                logFile << player << ": Shoot failed due to status or ammo." << std::endl;
                std::cout << "Attempted to shoot but could not due to status or ammo.\n";
            }
            break;
        }
        case ActionType::RotateLeft8:
            tank->rotateLeft8();  // Executes a 90-degree counterclockwise rotation
            logFile << player << ": Rotated left by 90 degrees." << std::endl;
            std::cout << "Rotated left by 90 degrees.\n";
            break;
        case ActionType::RotateRight8:
            tank->rotateRight8();  // Executes a 90-degree clockwise rotation
            logFile << player << ": Rotated right by 90 degrees." << std::endl;
            std::cout << "Rotated right by 90 degrees.\n";
            break;
        case ActionType::RotateLeft4:
            tank->rotateLeft4();  // Executes a 45-degree counterclockwise rotation
            logFile << player << ": Rotated left by 45 degrees." << std::endl;
            std::cout << "Rotated left by 45 degrees.\n";
            break;
        case ActionType::RotateRight4:
            tank->rotateRight4();  // Executes a 45-degree clockwise rotation
            logFile << player << ": Rotated right by 45 degrees." << std::endl;
            std::cout << "Rotated right by 45 degrees.\n";
            break;
        default:
            logFile << player << ": No action taken." << std::endl;
            std::cout << "No action taken.\n";
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
        logFile << "Shell fired at position (" << shellPos.x << ", " << shellPos.y << ")";
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