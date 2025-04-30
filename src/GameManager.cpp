#include "GameManager.h"
#include "Shell.h"
#include "Algorithm1.h"
#include "Algorithm2.h"
#include <iostream>
#include "Logger.h"
#include <unordered_set>


GameManager::GameManager(std::string inputFileName)
    : gameBoard(nullptr), currentStep(0), postAmmoSteps(80), inputFileName(inputFileName) {
    algorithm1 = std::make_unique<Algorithm1>();
    algorithm2 = std::make_unique<Algorithm2>();
    std::string outputFile = "output_" + inputFileName;
    logFile.open(outputFile);
}
GameManager::~GameManager() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

// reading the board from the input file
void GameManager::readBoard(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        exit(1);
    }

    std::string line;

    // שורה 1 – תיאור בלבד
    std::getline(file, line);

    // שורה 2 – MaxSteps
    std::getline(file, line);
    if (line.find("MaxSteps") == std::string::npos || line.find('=') == std::string::npos) {
        std::cerr << "Invalid MaxSteps line" << std::endl;
        exit(1);
    }
    try {
        maxSteps = std::stoi(line.substr(line.find('=') + 1));
    } catch (...) {
        std::cerr << "Failed to parse MaxSteps" << std::endl;
        exit(1);
    }

    // שורה 3 – NumShells
    std::getline(file, line);
    if (line.find("NumShells") == std::string::npos || line.find('=') == std::string::npos) {
        std::cerr << "Invalid NumShells line" << std::endl;
        exit(1);
    }
    try {
        numShells = std::stoi(line.substr(line.find('=') + 1));
    } catch (...) {
        std::cerr << "Failed to parse NumShells" << std::endl;
        exit(1);
    }

    // שורה 4 – Rows
    std::getline(file, line);
    int rows;
    if (line.find("Rows") == std::string::npos || line.find('=') == std::string::npos) {
        std::cerr << "Invalid Rows line" << std::endl;
        exit(1);
    }
    try {
        rows = std::stoi(line.substr(line.find('=') + 1));
    } catch (...) {
        std::cerr << "Failed to parse Rows" << std::endl;
        exit(1);
    }

    // שורה 5 – Cols
    std::getline(file, line);
    int cols;
    if (line.find("Cols") == std::string::npos || line.find('=') == std::string::npos) {
        std::cerr << "Invalid Cols line" << std::endl;
        exit(1);
    }
    try {
        cols = std::stoi(line.substr(line.find('=') + 1));
    } catch (...) {
        std::cerr << "Failed to parse Cols" << std::endl;
        exit(1);
    }

    // יצירת הלוח מחדש לפי המידות שהתקבלו
    gameBoard = std::make_unique<Board>(cols, rows);

    std::ofstream errorFile("input_errors.txt");
    bool hasTank1 = false, hasTank2 = false;

    for (int y = 0; y < rows; ++y) {
        std::string mapLine;
        if (!std::getline(file, mapLine)) {
            mapLine = std::string(cols, ' ');
            errorFile << "Warning: Missing row " << y << ", filled with spaces.\n";
        }

        if ((int)mapLine.length() > cols) {
            mapLine = mapLine.substr(0, cols);
            errorFile << "Warning: Row " << y << " longer than declared width, extra characters ignored.\n";
        }

        while ((int)mapLine.length() < cols) {
            mapLine += ' ';
            errorFile << "Warning: Row " << y << " shorter than declared width, filled with spaces.\n";
        }

        for (int x = 0; x < cols; ++x) {
            char c = mapLine[x];
            if (c == ' ') continue;

            switch (c) {
                case '#':
                    gameBoard->addObject(std::make_unique<Wall>(), x, y);
                    std::cout << "Added object '" << c << "' at (" << x << "," << y << ")" << std::endl;
                    break;
                case '@':
                    gameBoard->addObject(std::make_unique<Mine>(), x, y);
                    std::cout << "Added object '" << c << "' at (" << x << "," << y << ")" << std::endl;

                    break;
                case '1':
                    gameBoard->addObject(std::make_unique<Tank>('1', numShells, Direction(Direction::L), Position(x, y)), x, y);
                    hasTank1 = true;
                    break;
                case '2':
                    gameBoard->addObject(std::make_unique<Tank>('2', numShells, Direction(Direction::R), Position(x, y)), x, y);
                    hasTank2 = true;
                    break;
                default:
                    errorFile << "Warning: Unrecognized character '" << c << "' at (" << x << "," << y << "), treated as space.\n";
            }
        }
    }

    // שורות מיותרות מעבר לגובה
    std::string extra;
    while (std::getline(file, extra)) {
        if (!extra.empty()) {
            errorFile << "Warning: Extra row beyond declared height ignored.\n";
        }
    }

    errorFile.close();

    // בדיקה אם אין טנקים – סיום מיידי
    if (!hasTank1 && !hasTank2) {
        std::cerr << "Error: No tanks on map - game ends in tie immediately" << std::endl;
        exit(1);
    } else if (!hasTank1) {
        std::cerr << "Error: Player 1 has no tanks - Player 2 wins" << std::endl;
        exit(1);
    } else if (!hasTank2) {
        std::cerr << "Error: Player 2 has no tanks - Player 1 wins" << std::endl;
        exit(1);
    }

    // שליפת רשימות הטנקים לכל שחקן
    tanks1 = gameBoard->getTanks(1);
    tanks2 = gameBoard->getTanks(2);
}


// // the main loop of the game, as long as the game is not over it is moving the shells and tanks and check for collisions
// void GameManager::gameLoop() {
//     while (!checkGameOver()&&currentStep<=2000) {
//         // if both don't have ammo then every step we decrease the time of steps until draw
//         if (tank1->getAmmo() == 0 && tank2->getAmmo() == 0){
//             postAmmoSteps = postAmmoSteps - 1;
//         }
//         // we choose to move shells once in odd steps, and only shells, and then we check for collisions to see if shells hit other shells or tanks
//         if (currentStep % 2 != 0) {
//             moveShells();
//             checkCollisions();
//         } else { // if it's an even step
//             std::string turn= std::to_string((currentStep / 2 )+1);
//             Logger::debug("Turn : "+ turn);
//             logFile << "Turn : "+turn << std::endl;
//             IAlgorithm& algo1 = *algorithm1;
//             // we first move the shells and check for collisions
//             moveShells();
//             checkCollisions();
//             if (checkGameOver())
//             {
//                 break;
//             }
//             // then we move the tanks simoultansely and then check for collisions
//             executeTankAction(tank1, tank2, algo1);
//             IAlgorithm& algo2 = *algorithm2;
//             executeTankAction(tank2, tank1, algo2);
//             checkCollisions();
//         }
//         currentStep++;
//     }
//     logFile << "Game Over!" << std::endl;
//     Logger::debug("Game Over!");

//     if (!tank1->isAlive() && !tank2->isAlive()) {
//         logFile << "RESULT: Tie - Both tanks destroyed" << std::endl;
//         Logger::debug("RESULT: Tie - Both tanks destroyed");
//     } else if (!tank1->isAlive()) {
//         logFile << "RESULT: Player 2 wins - Player 1 destroyed" << std::endl;
//         Logger::debug("RESULT: Player 2 wins - Player 1 destroyed");

//     } else if (!tank2->isAlive()) {
//         logFile << "RESULT: Player 1 wins - Player 2 destroyed" << std::endl;
//         Logger::debug("RESULT: Player 1 wins - Player 2 destroyed");

//     } else if (tank1->getAmmo() == 0 && tank2->getAmmo() == 0 && postAmmoSteps <= 0) {
//         logFile << "RESULT: Tie - No ammo left and time ended" << std::endl;
//         Logger::debug("RESULT: Tie - No ammo left and time ended");
//     } else {
//         logFile << "RESULT: Tie - game reached the limit turns possible. No way of winning" << std::endl;
//         Logger::debug("RESULT: Tie - game reached the limit turns possible. No way of winning");
//     }



// }

// void GameManager::moveShells() {
//     auto& shells = gameBoard.getShells();
    
//     // go over all of the shells in order to move them
//     for (size_t i = 0; i < shells.size(); ++i) {
//         Shell* shell = shells[i];
//         if (!shell) continue; 

//         Position oldPos = shell->getPosition();
//         gameBoard.removeObject(shell, oldPos.x, oldPos.y);

//         shell->move(gameBoard.getWidth(), gameBoard.getHeight());

//         Position newPos = shell->getPosition();
//         //logFile << "Shell number " << shell->getId() << " fired at position (" << newPos.x << ", " << newPos.y << ")" << std::endl;
//         Logger::debug("Shell number " + std::to_string(shell->getId()) + " fired at position (" + std::to_string(newPos.x) + ", " + std::to_string(newPos.y) + ")");
//         gameBoard.addObject(shell, newPos.x, newPos.y);
//     }
// }


// // gets the tank, the enemy tank and the algorithm for the tank and does what the algorithm gave us if possible
// void GameManager::executeTankAction(Tank* tank, Tank* enemyTank, IAlgorithm& algo) {
//     if (!tank) return;
//     Action action = algo.nextAction(gameBoard, *tank, *enemyTank); // gets the next action from the alghorithm to peform on the tank
//     std::string player = (tank->getSymbol() == '1') ? "Player 1" : "Player 2";
//     Position initialPosition = tank->getPosition();
//     Direction direction = tank->getDirection();

//     // Logging to console for debugging
//     Logger::debug(player + " initiates action:");
//     // if the tank needs to do an action that is not shooting and he is in shooting cooldown, decrese the cooldown by one step
//     if (action.getType() != ActionType::Shoot && tank->getShootingStatus() > 0){
//         tank->decreaseShooting();
//     }
//     // if it's the first turn after a turn that the tank decided to move backward
//     if(tank->getBackwardStatus() == 3){
//         // if the tank decided to move forward then it cancels the backward cooldown and does nothing
//         if(action.getType()==ActionType::MoveForward){
//             tank->setBackward(0);
//             return;
//         }
//         else{
//             // decrease one turn from the cooldown
//             tank->decreaseBackward();
//             return;
//         }
//     }
//     // the tank finished the cooldown - the first turn was the turn he decided to move backward, then the turn that the status was 3, and not it's the third turn
//     if(tank->getBackwardStatus() == 2){
//         Position newPosition = initialPosition + direction.getOppositeDirection().toVector();
//         int xx=(newPosition.x+gameBoard.getWidth())%gameBoard.getWidth();
//         int yy=(newPosition.y+gameBoard.getHeight())%gameBoard.getHeight();
//         newPosition.setx(xx);
//         newPosition.sety(yy);
//         // does the moving backward and decrease the status to 1, in order to support multiple moving backwards in a row
//         tank->moveBackward(gameBoard.getWidth(), gameBoard.getHeight());
//         logFile << player << ": Moving backward now." << std::endl;
//                     logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//                     Logger::debug(player+ " Moving backward from (" + std::to_string(initialPosition.x) + ", " + std::to_string(initialPosition.y) +
//               ") to (" + std::to_string(newPosition.x) + ", " + std::to_string(newPosition.y) + ").");
//         tank->decreaseBackward();
//         return;
//     }
//     // if the tank did the moving backwards in the last turn and now does something else, decrease it to 0 so it can't do another moving backwards immediately
//     if(tank->getBackwardStatus() == 1 && action.getType()!=ActionType::MoveBackward){
//         tank->decreaseBackward();
//     }
//     // switch cases based on the actions that we got from the algorithm for the tank to execute
//     switch (action.getType()) {
//         case ActionType::MoveForward: {
//             Position newPosition = initialPosition + direction.toVector();
//             int xx=(newPosition.x+gameBoard.getWidth())%gameBoard.getWidth();
//             int yy=(newPosition.y+gameBoard.getHeight())%gameBoard.getHeight();
//             newPosition.setx(xx);
//             newPosition.sety(yy);
//             // if we can pass the cell and there are no mines or walls in the cell
//             if (gameBoard.isPassable(newPosition.x, newPosition.y)) {
//                 tank->moveForward(gameBoard.getWidth(), gameBoard.getHeight());
//                 logFile << player << ": MoveForward from (" << initialPosition.x << ", " << initialPosition.y << ") to (" << newPosition.x << ", " << newPosition.y << ")." << std::endl;
//                 logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//                 Logger::debug(player+ " Moved forward from (" + std::to_string(initialPosition.x) + ", " + std::to_string(initialPosition.y) +
//               ") to (" + std::to_string(newPosition.x) + ", " + std::to_string(newPosition.y) + ").");
//             // we can't move forward to the cell
//             } else {
//                 logFile << player << ": Bad step - tried to move into wall or blocked cell." << std::endl;
//                 Logger::debug(player+ " MoveForward failed due to obstruction.Bad step.");
//             }
//             break;
//         }

//         case ActionType::MoveBackward: {
//             // we start initiate the moving backwards but doesn't do it immediately
//             Position newPosition = initialPosition + direction.getOppositeDirection().toVector();
//             int xx=(newPosition.x+gameBoard.getWidth())%gameBoard.getWidth();
//             int yy=(newPosition.y+gameBoard.getHeight())%gameBoard.getHeight();
//             newPosition.setx(xx);
//             newPosition.sety(yy);
//             if (tank->getBackwardStatus() == 0) {
//                 if (gameBoard.isPassable(newPosition.x, newPosition.y)) {
//                     tank->setBackward(3);
//                     logFile << player << ": Started MoveBackward process." << std::endl;
//                     logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//                     Logger::debug(player+ " Started MoveBackward process from (" + std::to_string(initialPosition.x) + ", " + std::to_string(initialPosition.y) +
//               ") to (" + std::to_string(newPosition.x) + ", " + std::to_string(newPosition.y) + ").");
//                 // we can't move backwards
//                 } else {
//                     logFile << player << ": MoveBackward failed due to obstruction." << std::endl;
//                     Logger::debug(player+ " MoveBackward failed due to obstruction.");
//                 }
//             }
//             // if the status is 1, it means that in the last turn we did the moving backwards itself and we can do it again without cooldown
//             else if(tank->getBackwardStatus() == 1){
//                 if (gameBoard.isPassable(newPosition.x, newPosition.y)) {
//                     tank->moveBackward(gameBoard.getWidth(), gameBoard.getHeight());
//                     logFile << player << ": Moving Backward now." << std::endl;
//                     logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//                     Logger::debug(player+ " Moving backward from (" + std::to_string(initialPosition.x) + ", " + std::to_string(initialPosition.y) +
//               ") to (" + std::to_string(newPosition.x) + ", " + std::to_string(newPosition.y) + ").");
//                 }
//                 // we can't move backwards
//                 }
//                 else {
//                     logFile << player << ": MoveBackward failed due to obstruction." << std::endl;
//                     Logger::debug(player+ " MoveBackward failed due to obstruction.");
//                 }
//                 break;
//             }

//         case ActionType::Shoot: {
//             // the tank can shoot - it has enough ammo and not in cooldown
//             if (tank->getShootingStatus() == 0 && tank->getAmmo() > 0) {
//                 Position shootPosition = initialPosition + direction.toVector();
//                 int x = (shootPosition.x + gameBoard.getWidth()) % gameBoard.getWidth();
//                 int y = (shootPosition.y + gameBoard.getHeight()) % gameBoard.getHeight();
//                 tank->shoot();
//                 // creating a new shell in the direction of the tank - the shell is created one cell ahead
//                 Shell* newShell = new Shell(Position(x, y), direction, tank->getSymbol());
//                 gameBoard.addObject(newShell, x, y);
//                 logFile << player << ": Shoot from (" << initialPosition.x << ", " << initialPosition.y << ") to (" << x << ", " << y << ") in direction " << direction.getDirection() << "." << std::endl;
//                 logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//                 Logger::debug(player+ " Shot from (" + std::to_string(initialPosition.x) + ", " + std::to_string(initialPosition.y) +
//               ") to (" + std::to_string(x) + ", " + std::to_string(y) + ") in direction " + std::to_string(direction.getDirection()) + ".");

//             } else {
//                 // if we are in shooting cooldown, then decrease it by 1
//                 if (tank->getShootingStatus() > 0){
//                     tank->decreaseShooting();
//                 }
//                 logFile << player << ": Bad step - attempted to shoot with no ammo." << std::endl;
//                 Logger::debug(player+ " Shoot failed due to status or ammo.Bad Step");
//             }
//             break;
//         }
//         // rotate actions - can't go wrong
//         case ActionType::RotateLeft8:
//             tank->rotateLeft8();  // Executes a 45-degree counterclockwise rotation
//             logFile << player << ": Rotated left by 45 degrees." << std::endl;
//             logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//             Logger::debug(player+ " Rotated left by 90 degrees.");
//             break;
//         case ActionType::RotateRight8:
//             tank->rotateRight8();  // Executes a 90-degree clockwise rotation
//             logFile << player << ": Rotated right by 45 degrees." << std::endl;
//             logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//             Logger::debug(player+ " Rotated right by 90 degrees.");
//             break;
//         case ActionType::RotateLeft4:
//             tank->rotateLeft4();  // Executes a 90-degree counterclockwise rotation
//             logFile << player << ": Rotated left by 90 degrees." << std::endl;
//             logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//             Logger::debug(player+ " Rotated left by 45 degrees.");
//             break;
//         case ActionType::RotateRight4:
//             tank->rotateRight4();  // Executes a 90-degree clockwise rotation
//             logFile << player << ": Rotated right by 90 degrees." << std::endl;
//             logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//             Logger::debug(player+ " Rotated right by 45 degrees.");
//             break;
//         default:
//             logFile << player << ": No action taken." << std::endl;
//             logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//             Logger::debug(player+ " No action taken.");
//             break;
//     }
// }

// // a function that check if there are collsions between shells and walls, shells and shells, mines with tanks, tanks with tanks and shells with tanks
// void GameManager::checkCollisions() {
//     auto& shells = gameBoard.getShells();
//     Position posTank1 = tank1->getPosition();
//     Position posTank2 = tank2->getPosition();
//     // if the tank are colided between them
//     if (posTank1 == posTank2 && tank1->isAlive() && tank2->isAlive()) {
//         tank1->hit();
//         tank2->hit();
//         gameBoard.removeObject(tank1, posTank1.x, posTank1.y);
//         gameBoard.removeObject(tank2, posTank2.x, posTank2.y);
//         logFile << "Collision: Tank1 and Tank2 collided -> Tie (both destroyed)" << std::endl;
//         Logger::debug("Collision: Tank1 and Tank2 collided -> Tie (both destroyed)");
//         return;
//     }
//     // checking if in the slot of any of the tanks there's a mine
//     for (Tank* tank : {tank1, tank2}) {
//         Position pos = tank->getPosition();
//         CellSlot& slot = gameBoard.getSlot(pos.x, pos.y);
//         if (slot.getMine()) {
//             // removing both tanks and mines
//             gameBoard.removeObject(tank, pos.x, pos.y);
//             gameBoard.removeObject(slot.getMine(), pos.x, pos.y);
//             tank->hit();
//             std::string player = (tank == tank1 ? "Player 1" : "Player 2");
//             logFile << "Mine: " << player << " hit a mine and was destroyed. Enemy wins." << std::endl;
//             Logger::debug("Mine: " + player + " hit a mine and was destroyed. Enemy wins.");
//             return;
//         }
//     }
//     // now we check for each shell if it hits another shell, wall or tank
//     std::unordered_set<Shell*> shellsToRemove;
//     std::vector<Shell*> shellsCopy = shells;
//     for (Shell* shell : shellsCopy) {
//         Position shellPos = shell->getPosition();
//         //logFile << "Shell number " << shell->getId() << " fired at position (" << shellPos.x << ", " << shellPos.y << ")" << std::endl;
//         Logger::debug("Shell number " + std::to_string(shell->getId()) + " fired at position (" + std::to_string(shellPos.x) + ", " + std::to_string(shellPos.y) + ")");

//         bool hitTank1 = (shellPos == posTank1);
//         bool hitTank2 = (shellPos == posTank2);

//         if (hitTank1 && hitTank2) {
//             // Both tanks are hit, it's a tie
//             tank1->hit();
//             tank2->hit();
//             gameBoard.removeObject(tank1, posTank1.x, posTank1.y);
//             gameBoard.removeObject(tank2, posTank2.x, posTank2.y);
//             logFile << "Shell: Player 1 was hit by shell and destroyed." << std::endl;
//             Logger::debug("Shell: Player 1 was hit by shell and destroyed.");
//             logFile << "Shell: Player 2 was hit by shell and destroyed." << std::endl;
//             Logger::debug("Shell: Player 2 was hit by shell and destroyed.");
//             break; // End game on a tie
//         } else if (hitTank1) {
//             // Only Tank1 is hit
//             tank1->hit();
//             gameBoard.removeObject(tank1, posTank1.x, posTank1.y);
//             logFile << "Shell: Player 1 was hit by shell and destroyed." << std::endl;
//             Logger::debug("Shell: Player 1 was hit by shell and destroyed.");
//         } else if (hitTank2) {
//             // Only Tank2 is hit
//             tank2->hit();
//             gameBoard.removeObject(tank2, posTank2.x, posTank2.y);
//             logFile << "Shell: Player 2 was hit by shell and destroyed." << std::endl;
//             Logger::debug("Shell: Player 2 was hit by shell and destroyed.");
//         }
//         // Remove the shell if it hits any tank
//         if (hitTank1 || hitTank2) {
//             gameBoard.removeObject(shell, shellPos.x, shellPos.y);
//             delete shell;
            
//         }
//         CellSlot& slot = gameBoard.getSlot(shellPos.x, shellPos.y);
//         if (slot.getWall()) {
//             int hp = slot.getWall()->onHit();
//             logFile << "Shell hit wall at position (" << shellPos.x << ", " << shellPos.y << "). Wall health is now " << hp << std::endl;
//             Logger::debug("Shell hit wall at position (" + std::to_string(shellPos.x) + ", " + std::to_string(shellPos.y) + "). Wall health is now " + std::to_string(hp));

//             // removing the shell because it hit the wall
//             gameBoard.removeObject(shell, shellPos.x, shellPos.y);
//             delete shell;
//             // if the wall is destroyed, remove the wall
//             if (hp <= 0) {
//                 gameBoard.removeObject(slot.getWall(), shellPos.x, shellPos.y);
//                 logFile << "Wall destroyed." << std::endl;
//                 Logger::debug("Wall destroyed.");
//             }
//             continue;
//         }

//  // בודק אם יש פגזים אחרים באותו תא
//     for (Shell* otherShell : slot.getShells()) {
//         if (otherShell != shell && shellPos == otherShell->getPosition()) {
//             shellsToRemove.insert(shell);
//             shellsToRemove.insert(otherShell);
//             logFile << "Shell: Two shells collided and were destroyed." << std::endl;
//             Logger::debug("Shell: Two shells collided and were destroyed.");
//             break;
//         }
//     }
// }

//     for (Shell* shell : shellsToRemove) {
//         Position pos = shell->getPosition();
//         gameBoard.removeObject(shell, pos.x, pos.y);
//         delete shell;
//     }
// }


// // checks if the game is over if we finished the number of turns after both tanks lost all ammo, or if one of the tanks is dead
// bool GameManager::checkGameOver() {
//     if (tank1->getAmmo() == 0 && tank2->getAmmo() == 0 && postAmmoSteps-- <= 0)
//         return true;
//     if (!tank1->isAlive() || !tank2->isAlive())
//         return true;
//     return false;
// }