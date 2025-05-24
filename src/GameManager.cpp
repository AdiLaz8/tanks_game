#include "GameManager.h"
#include "Shell.h"
#include "Algorithm1.h"
#include "Algorithm2.h"
#include <iostream>
#include "Logger.h"
#include <unordered_set>




GameManager::GameManager(const PlayerFactory& pf, const TankAlgorithmFactory& tf)
    : playerFactory(pf), tankAlgoFactory(tf), currentStep(0) {}
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

    std::string outputFile = "output_" + filename.substr(filename.find_last_of("/\\") + 1);
    logFile.open(outputFile);
    if (!logFile.is_open()) {
        std::cerr << "Error: Cannot open output file: " << outputFile << std::endl;
        exit(1);
    }

    std::string line;
    std::getline(file, line); // Header line
    logFile << "Header: " << line << std::endl;

    // MaxSteps
    std::getline(file, line);
    if (line.find("MaxSteps") == std::string::npos || line.find('=') == std::string::npos) {
        logFile << "Error: MaxSteps line invalid" << std::endl;
        exit(1);
    }
    try {
        maxSteps = static_cast<size_t>(std::stoi(line.substr(line.find('=') + 1)));
    } catch (...) {
        logFile << "Error: Failed to parse MaxSteps" << std::endl;
        exit(1);
    }

    // NumShells
    std::getline(file, line);
    if (line.find("NumShells") == std::string::npos || line.find('=') == std::string::npos) {
        logFile << "Error: NumShells line invalid" << std::endl;
        exit(1);
    }
    try {
        numShells = static_cast<size_t>(std::stoi(line.substr(line.find('=') + 1)));
    } catch (...) {
        logFile << "Error: Failed to parse NumShells" << std::endl;
        exit(1);
    }

    // Rows
    std::getline(file, line);
    size_t rows;
    if (line.find("Rows") == std::string::npos || line.find('=') == std::string::npos) {
        logFile << "Error: Rows line invalid" << std::endl;
        exit(1);
    }
    try {
        rows = static_cast<size_t>(std::stoi(line.substr(line.find('=') + 1)));
    } catch (...) {
        logFile << "Error: Failed to parse Rows" << std::endl;
        exit(1);
    }

    // Cols
    std::getline(file, line);
    size_t cols;
    if (line.find("Cols") == std::string::npos || line.find('=') == std::string::npos) {
        logFile << "Error: Cols line invalid" << std::endl;
        exit(1);
    }
    try {
        cols = static_cast<size_t>(std::stoi(line.substr(line.find('=') + 1)));
    } catch (...) {
        logFile << "Error: Failed to parse Cols" << std::endl;
        exit(1);
    }

    gameBoard = std::make_unique<Board>(cols, rows);
    player1 = playerFactory.create(1, cols, rows, maxSteps, numShells);
    player2 = playerFactory.create(2, cols, rows, maxSteps, numShells);

    std::ofstream errorFile("input_errors.txt");
    bool hasTank1 = false, hasTank2 = false;
    int tankIndex1 = 0;
    int tankIndex2 = 0;

    for (size_t y = 0; y < rows; ++y) {
        std::string mapLine;
        if (!std::getline(file, mapLine)) {
            mapLine = std::string(cols, ' ');
            errorFile << "Warning: Missing row " << y << ", filled with spaces.\n";
        }

        if ((int)mapLine.length() > static_cast<int>(cols)) {
            mapLine = mapLine.substr(0, cols);
            errorFile << "Warning: Row " << y << " too long.\n";
        }

        while (mapLine.length() < cols) {
            mapLine += ' ';
            errorFile << "Warning: Row " << y << " too short, padded with spaces.\n";
        }

        for (size_t x = 0; x < cols; ++x) {
            char c = mapLine[x];
            if (c == ' ') continue;

            switch (c) {
                case '#':
                    gameBoard->addObject(std::make_unique<Wall>(), x, y);
                    break;
                case '@':
                    gameBoard->addObject(std::make_unique<Mine>(), x, y);
                    break;
                case '1': {
                    gameBoard->addObject(std::make_unique<Tank>('1', Direction(Direction::L), Position(x, y)), x, y);
                    hasTank1 = true;
                    std::unique_ptr<TankAlgorithm> base = tankAlgoFactory.create(1, tankIndex1);
                    Tank* t = gameBoard->getSlot(x, y).getTank();
                    auto raw = dynamic_cast<MyTankAlgorithm*>(base.get());
                    if (t && raw) {
                        std::unique_ptr<MyTankAlgorithm> algo(static_cast<MyTankAlgorithm*>(base.release()));
                        tankPairs.emplace_back(std::move(algo), t);
                        logFile << "Player 1 tank placed at (" << x << "," << y << ")" << std::endl;
                        tankMap1++;
                    } else {
                        logFile << "Error: Failed to create tank algorithm for Player 1 at index " << tankIndex1 << std::endl;
                        exit(1);
                    }
                    tankIndex1++;
                    break;
                }
                case '2': {
                    gameBoard->addObject(std::make_unique<Tank>('2', Direction(Direction::L), Position(x, y)), x, y);
                    hasTank2 = true;
                    std::unique_ptr<TankAlgorithm> base = tankAlgoFactory.create(2, tankIndex2);
                    Tank* t = gameBoard->getSlot(x, y).getTank();
                    auto raw = dynamic_cast<MyTankAlgorithm*>(base.get());
                    if (t && raw) {
                        std::unique_ptr<MyTankAlgorithm> algo(static_cast<MyTankAlgorithm*>(base.release()));
                        tankPairs.emplace_back(std::move(algo), t);
                        logFile << "Player 2 tank placed at (" << x << "," << y << ")" << std::endl;
                        tankMap2++;

                    } else {
                        logFile << "Error: Failed to create tank algorithm for Player 2 at index " << tankIndex2 << std::endl;
                        exit(1);
                    }
                    tankIndex2++;
                    break;
                }
                default:
                    errorFile << "Warning: Unknown char '" << c << "' at (" << x << "," << y << ")\n";
            }
        }
    }

    std::string extra;
    while (std::getline(file, extra)) {
        if (!extra.empty()) errorFile << "Warning: Extra row ignored.\n";
    }
    errorFile.close();

    if (!hasTank1 && !hasTank2) {
        logFile << "Error: No tanks on map - tie." << std::endl;
        exit(1);
    } else if (!hasTank1) {
        logFile << "Error: Player 1 has no tanks - Player 2 wins." << std::endl;
        exit(1);
    } else if (!hasTank2) {
        logFile << "Error: Player 2 has no tanks - Player 1 wins." << std::endl;
        exit(1);
    }
}



void GameManager::moveShells() {
    auto& shells = gameBoard->getShells();
    
    // go over all of the shells in order to move them
    for (size_t i = 0; i < shells.size(); ++i) {
        Shell* shell = shells[i];
        if (!shell) continue; 

        Position oldPos = shell->getPosition();
        gameBoard->removeObject(shell, oldPos.getx(), oldPos.gety());

        shell->move(gameBoard->getWidth(), gameBoard->getHeight());

        Position newPos = shell->getPosition();
        //logFile << "Shell number " << shell->getId() << " fired at position (" << newPos.x << ", " << newPos.y << ")" << std::endl;
        Logger::debug("Shell number " + std::to_string(shell->getId()) + " fired at position (" + std::to_string(newPos.getx()) + ", " + std::to_string(newPos.gety()) + ")");
        gameBoard->addObject(std::unique_ptr<Cell>(shell), newPos.getx(), newPos.gety());
    }
}


void GameManager::gameLoop() {
    while (!checkGameOver()) {
        if (currentStep % 2 != 0) {
            moveShells();
            checkCollisions();
        } else {
            std::string turn = std::to_string(currentStep / 2 + 1);
            Logger::debug("Turn : " + turn);
            logFile << "Turn : " + turn << std::endl;

            moveShells();
            checkCollisions();
            if (checkGameOver()) break;

            auto boardView = buildBoardMatrix();
            MySatelliteView satellite(boardView);

            for (auto& [algoPtr, tank] : tankPairs) {
                if (! tank->isAlive()) continue;

                ActionRequest request = algoPtr->getAction();
                logFile << "Player " << tank->getSymbol() << ": Requested action - " << static_cast<int>(request) << std::endl;
                if (request == ActionRequest::GetBattleInfo) {
                    logFile << "Player " << tank->getSymbol() << ": GetBattleInfo triggered\n";
                    satellite.setPosition(tank->getPosition());
                    if (tank->getSymbol() == '1') {
                        player1->updateTankWithBattleInfo(*algoPtr, satellite);
                    } else {
                        player2->updateTankWithBattleInfo(*algoPtr, satellite);
                    }
                } else {
                    executeAction(request, *algoPtr, tank);
                    logFile << "Player " << tank->getSymbol() << ": doing action - " << static_cast<int>(request) << std::endl;

                }
            }

            checkCollisions();
        }
        currentStep++;
    }

    logGameResult();
}


void GameManager::executeAction(const ActionRequest& req, MyTankAlgorithm& algo, Tank* tank) {
    Position pos = tank->getPosition();
    Direction dir = tank->getDirection();
    std::string player = (tank->getSymbol() == '1') ? "Player 1" : "Player 2";

    Logger::debug(player + " initiates action:");
    if (algo.getBackwardStatus() == 3 && req != ActionRequest::MoveForward) {
        algo.decreaseBackward();
        return;
    }
    if (algo.getBackwardStatus() == 3 && req == ActionRequest::MoveForward) {
        algo.setBackward(0);
        return;
    }
    if (algo.getBackwardStatus() == 2) {
        Position back = pos + dir.getOppositeDirection().toVector();
        wrapPosition(back);
        tank->moveBackward(gameBoard->getWidth(), gameBoard->getHeight());
        logFile << player << ": Moving backward now." << std::endl;
        Logger::debug(player + " moved backward.");
        algo.decreaseBackward();
        return;
    }
    if (algo.getBackwardStatus() == 1 && req != ActionRequest::MoveBackward) {
        algo.decreaseBackward();
    }
    switch (req) {
        case ActionRequest::MoveForward: {
            Position next = pos + dir.toVector();
            wrapPosition(next);
            if (gameBoard->isPassable(next.getx(), next.gety())) {
                tank->moveForward(gameBoard->getWidth(), gameBoard->getHeight());
                logFile << player << ": MoveForward to (" << next.getx() << ", " << next.gety() << ")" << std::endl;
            } else {
                logFile << player << ": Bad step - blocked forward." << std::endl;
            }
            break;
        }
        case ActionRequest::MoveBackward: {
            Position back = pos + dir.getOppositeDirection().toVector();
            wrapPosition(back);
            if (algo.getBackwardStatus() == 0 && gameBoard->isPassable(back.getx(), back.gety())) {
                algo.setBackward(3);
                logFile << player << ": Started MoveBackward process." << std::endl;
            }
            break;
        }
        case ActionRequest::Shoot: {
            Position shoot = pos + dir.toVector();
            wrapPosition(shoot);
            auto shell = std::make_unique<Shell>(shoot, dir, tank->getSymbol());
            gameBoard->addObject(std::move(shell), shoot.getx(), shoot.gety());
            logFile << player << ": Shoot from (" << pos.getx() << ", " << pos.gety() << ") to (" << shoot.getx() << ", " << shoot.gety() << ")" << std::endl;
            break;
        }
        case ActionRequest::RotateLeft45: tank->rotateLeft8(); break;
        case ActionRequest::RotateRight45: tank->rotateRight8(); break;
        case ActionRequest::RotateLeft90: tank->rotateLeft4(); break;
        case ActionRequest::RotateRight90: tank->rotateRight4(); break;
        default:
            logFile << player << ": No action taken." << std::endl;
            break;
    }
    logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
}
void GameManager::wrapPosition(Position& pos) {
    pos.setx((pos.getx() + gameBoard->getWidth()) % gameBoard->getWidth());
    pos.sety((pos.gety() + gameBoard->getHeight()) % gameBoard->getHeight());
}

std::vector<std::vector<char>> GameManager::buildBoardMatrix() {
    size_t h = gameBoard->getHeight(), w = gameBoard->getWidth();
    std::vector<std::vector<char>> board(h, std::vector<char>(w, ' '));
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x) {
            const CellSlot& slot = gameBoard->getSlot(x, y);
            if (slot.getTank()) board[y][x] = slot.getTank()->getSymbol();
            else if (slot.getWall()) board[y][x] = '#';
            else if (slot.getMine()) {
                if(!slot.getShells().empty()){
                    board[y][x] = '*';
                }
                else{board[y][x] = '@';}}
            else if (!slot.getShells().empty()) board[y][x] = '*';
        }
    }
    return board;
}
void GameManager::checkCollisions() {
    std::vector<Shell*> toRemove;

    for (Shell* shell : gameBoard->getShells()) {
        Position pos = shell->getPosition();
        CellSlot& slot = gameBoard->getSlot(pos.getx(), pos.gety());

        if (Tank* tank = slot.getTank()) {
            tank->Hit();
            if (tank->getSymbol() == '1') tankMap1--;
            else if (tank->getSymbol() == '2') tankMap2--;
            tankPairs.erase(std::remove_if(tankPairs.begin(), tankPairs.end(),
                [tank](const auto& pair) { return pair.second == tank; }), tankPairs.end());
            gameBoard->removeObject(tank, pos.getx(), pos.gety());
            logFile << "Shell: Tank " << tank->getSymbol() << " destroyed at (" << pos.getx() << "," << pos.gety() << ")" << std::endl;
            Logger::debug("Shell destroyed tank at position (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + ")");
            toRemove.push_back(shell);
            continue;
        }

        if (Wall* wall = slot.getWall()) {
            int hp = wall->onHit();
            logFile << "Shell hit wall at (" << pos.getx() << "," << pos.gety() << "). Wall HP: " << hp << std::endl;
            if (hp <= 0) {
                gameBoard->removeObject(wall, pos.getx(), pos.gety());
                logFile << "Wall destroyed at (" << pos.getx() << "," << pos.gety() << ")" << std::endl;
            }
            toRemove.push_back(shell);
            continue;
        }

        for (Shell* other : slot.getShells()) {
            if (other != shell && other->getPosition() == pos) {
                logFile << "Shells collided at (" << pos.getx() << "," << pos.gety() << ")" << std::endl;
                toRemove.push_back(shell);
                toRemove.push_back(other);
                break;
            }
        }
    }

    for (Shell* s : toRemove) {
        Position pos = s->getPosition();
        gameBoard->removeObject(s, pos.getx(), pos.gety());
    }

    for (auto it = tankPairs.begin(); it != tankPairs.end(); ) {
        Tank* tank = it->second;
        if (!tank->isAlive()) {
            ++it;
            continue;
        }
        Position pos = tank->getPosition();
        CellSlot& slot = gameBoard->getSlot(pos.getx(), pos.gety());
        if (slot.getMine()) {
            tank->Hit();
            if (tank->getSymbol() == '1') tankMap1--;
            else if (tank->getSymbol() == '2') tankMap2--;
            gameBoard->removeObject(tank, pos.getx(), pos.gety());
            gameBoard->removeObject(slot.getMine(), pos.getx(), pos.gety());
            logFile << "Mine: Player " << tank->getSymbol() << " tank hit a mine at (" << pos.getx() << "," << pos.gety() << ")" << std::endl;
            it = tankPairs.erase(it);
        } else {
            ++it;
        }
    }

}
bool GameManager::checkGameOver() const {
    return tankMap1==0 || tankMap2==0|| currentStep >= maxSteps;
}

void GameManager::logGameResult() {
    logFile << "Game Over!" << std::endl;
    Logger::debug("Game Over!");

    if (tankMap1==0 && tankMap2==0)
        logFile << "RESULT: Tie - Both players lost all tanks" << std::endl;
    else if (tankMap1==0)
        logFile << "RESULT: Player 2 wins - Player 1 eliminated" << std::endl;
    else if (tankMap2==0)
        logFile << "RESULT: Player 1 wins - Player 2 eliminated" << std::endl;
    else
        logFile << "RESULT: Tie - Reached max steps" << std::endl;
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
//     if (action.getType() != ActionRequest::Shoot && tank->getShootingStatus() > 0){
//         tank->decreaseShooting();
//     }
//     // if it's the first turn after a turn that the tank decided to move backward
//     if(tank->getBackwardStatus() == 3){
//         // if the tank decided to move forward then it cancels the backward cooldown and does nothing
//         if(action.getType()==ActionRequest::MoveForward){
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
//     if(tank->getBackwardStatus() == 1 && action.getType()!=ActionRequest::MoveBackward){
//         tank->decreaseBackward();
//     }
//     // switch cases based on the actions that we got from the algorithm for the tank to execute
//     switch (action.getType()) {
//         case ActionRequest::MoveForward: {
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

//         case ActionRequest::MoveBackward: {
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

//         case ActionRequest::Shoot: {
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
//         case ActionRequest::RotateLeft8:
//             tank->rotateLeft8();  // Executes a 45-degree counterclockwise rotation
//             logFile << player << ": Rotated left by 45 degrees." << std::endl;
//             logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//             Logger::debug(player+ " Rotated left by 90 degrees.");
//             break;
//         case ActionRequest::RotateRight8:
//             tank->rotateRight8();  // Executes a 90-degree clockwise rotation
//             logFile << player << ": Rotated right by 45 degrees." << std::endl;
//             logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//             Logger::debug(player+ " Rotated right by 90 degrees.");
//             break;
//         case ActionRequest::RotateLeft4:
//             tank->rotateLeft4();  // Executes a 90-degree counterclockwise rotation
//             logFile << player << ": Rotated left by 90 degrees." << std::endl;
//             logFile << player << ": Direction is now " << tank->getDirection().getDirection() << std::endl;
//             Logger::debug(player+ " Rotated left by 45 degrees.");
//             break;
//         case ActionRequest::RotateRight4:
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