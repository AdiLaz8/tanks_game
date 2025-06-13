#include "GameManager.h"
#include "ActionRequestUtils.h"
#include "Shell.h"
#include <iostream>
#include "Logger.h"
#include <unordered_set>

GameManager::GameManager(const PlayerFactory& pf, const TankAlgorithmFactory& tf)
    : playerFactory(pf), tankAlgoFactory(tf), currentStep(0) {}

GameManager::~GameManager() {
    if (simpleOutput.is_open()) {
        simpleOutput.close();
    }
}
// reading the board header from the input file (cols, rows, name, maxSteps, numShells)
void GameManager::readBoardHeader(const std::string& filename, std::ifstream& file, size_t& rows, size_t& cols) {
    file.open(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        exit(1);
    }
    std::string outputFile = "output_" + filename.substr(filename.find_last_of("/\\") + 1);
    simpleOutput.open(outputFile);
    if (!simpleOutput.is_open()) {
        std::cerr << "Error: Cannot open output file: " << outputFile << std::endl;
        exit(1);
    }
    std::string line;
    std::getline(file, line); // Header line
    Logger::debug("Header: " + line);
    // MaxSteps
    std::getline(file, line);
    if (line.find("MaxSteps") == std::string::npos || line.find('=') == std::string::npos) {
        Logger::debug("Error: MaxSteps line invalid");
        exit(1);
    }
    try {
        maxSteps = static_cast<size_t>(std::stoi(line.substr(line.find('=') + 1)));
    } catch (...) {
        Logger::debug("Error: Failed to parse MaxSteps");
        exit(1);
    }
    // NumShells
    std::getline(file, line);
    if (line.find("NumShells") == std::string::npos || line.find('=') == std::string::npos) {
        Logger::debug("Error: NumShells line invalid");
        exit(1);
    }
    try {
        numShells = static_cast<size_t>(std::stoi(line.substr(line.find('=') + 1)));
    } catch (...) {
        Logger::debug("Error: Failed to parse NumShells");
        exit(1);
    }
    // Rows
    std::getline(file, line);
    if (line.find("Rows") == std::string::npos || line.find('=') == std::string::npos) {
        Logger::debug("Error: Rows line invalid");
        exit(1);
    }
    try {
        rows = static_cast<size_t>(std::stoi(line.substr(line.find('=') + 1)));
    } catch (...) {
        Logger::debug("Error: Failed to parse Rows");
        exit(1);
    }
    // Cols
    std::getline(file, line);
    if (line.find("Cols") == std::string::npos || line.find('=') == std::string::npos) {
        Logger::debug("Error: Cols line invalid");
        exit(1);
    }
    try {
        cols = static_cast<size_t>(std::stoi(line.substr(line.find('=') + 1)));
    } catch (...) {
        Logger::debug("Error: Failed to parse Cols");
        exit(1);
    }
    Logger::debug("Board size: " + std::to_string(rows) + " rows, " + std::to_string(cols) + " cols");
}

// reading the board from the input file
void GameManager::readBoard(const std::string& filename) {
    std::ifstream file;
    size_t rows, cols;
    readBoardHeader(filename, file, rows, cols);
    gameBoard = std::make_unique<Board>(cols, rows);
    player1 = playerFactory.create(1, cols, rows, maxSteps, numShells);
    player2 = playerFactory.create(2, cols, rows, maxSteps, numShells);
    std::ofstream errorFile("input_errors.txt");
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
                case '1':
                case '2': {
                    int playerId = (c == '1') ? 1 : 2;
                    Direction dir = (c == '1') ? Direction(Direction::L) : Direction(Direction::R);
                    int& tankIdx = (c == '1') ? tankIndex1 : tankIndex2;
                    gameBoard->addObject(std::make_unique<Tank>(c, dir, Position(x, y)), x, y);
                    std::unique_ptr<TankAlgorithm> algo = tankAlgoFactory.create(playerId, tankIdx);
                    Tank* t = gameBoard->getSlot(x, y).getTank();
                    t->setTankIndex(tankIdx);
                    t->setRemainingShells(numShells);
                    tankPairs.emplace_back(std::move(algo), t);
                    Logger::debug(t->getFullIdString() + " tank placed at (" + std::to_string(x) + "," + std::to_string(y) + ")");
                    tankIdx++;
                    break;
                }
                default:
                    errorFile << "Warning: Unknown char '" << c << "' at (" << x << "," << y << ")\n";
            }
        }
    }
    populateTankOrderAndLog(rows, cols);
    finalizeBoardReading(file, errorFile, tankIndex1, tankIndex2);
}

// Setting up the tanks order for the output file
void GameManager::populateTankOrderAndLog(size_t rows, size_t cols) {
    int birthIndex = 0;
    for (size_t y = 0; y < rows; ++y) {
        for (size_t x = 0; x < cols; ++x) {
            Tank* t = gameBoard->getSlot(x, y).getTank();
            if (t) {
                tanksOrderedByBirth.push_back(t);
                t->setBirthIndex(birthIndex++);
            }
        }
    }
    for (Tank* t : tanksOrderedByBirth) {
        TankLogInfo info;
        info.symbol = t->getSymbol();
        tankLog.push_back(info);
    }
}

void GameManager::finalizeBoardReading(std::ifstream& file, std::ofstream& errorFile, int& tankIndex1, int& tankIndex2) {
    // checking if there are too many lines
    std::string extra;
    while (std::getline(file, extra)) {
        if (!extra.empty()) errorFile << "Warning: Extra row ignored.\n";
    }
    errorFile.close();

    // checking if there are no tanks on the map/only tanks of one player
    if (tankIndex1 == 0 && tankIndex2 == 0) {
        Logger::debug("Error: No tanks on map - tie.");
        simpleOutput << "Error: No tanks on map - tie." << std::endl;
        exit(1);
    } else if (tankIndex1 == 0) {
        Logger::debug("Error: Player 1 has no tanks - Player 2 wins.");
        simpleOutput << "Error: Player 1 has no tanks - Player 2 wins." << std::endl;
        exit(1);
    } else if (tankIndex2 == 0) {
        Logger::debug("Error: Player 2 has no tanks - Player 1 wins.");
        simpleOutput << "Error: Player 2 has no tanks - Player 1 wins." << std::endl;
        exit(1);
    }
}


void GameManager::moveShells() {
    std::vector<Shell*> copy = gameBoard->getShells(); 
    for (Shell* shell : copy) {
        Position oldPos = shell->getPosition();
        Position newPos = oldPos + shell->getDirection().toVector();
        wrapPosition(newPos);
        gameBoard->moveShellTo(shell, oldPos.getx(), oldPos.gety(), newPos.getx(), newPos.gety());
        shell->setPosition(newPos);
    }
}

void GameManager::gameLoop() {
    while (!checkGameOver()) {
        if (currentStep % 2 != 0) {
            moveShells();
            checkCollisions();
            for (size_t i = 0; i < tankLog.size(); ++i) {
                std::string action;
                if (!tankLog[i].isAlive && tankLog[i].wasKilledThisTurn) { action = tankLog[i].lastAction + " (killed)"; }
                else if (!tankLog[i].isAlive) { action = "killed"; }
                else { action = tankLog[i].lastAction; }
                simpleOutput << action;
                if (i + 1 < tankLog.size()) simpleOutput << ", ";
            }
            simpleOutput << std::endl;
            for (auto& info : tankLog) { info.wasKilledThisTurn = false; }
        } else {
            std::string turn = std::to_string(currentStep / 2 + 1);
            Logger::debug("Turn : " + turn);
            moveShells();
            checkCollisions();
            if (checkGameOver()) break;
            auto boardView = buildBoardMatrix(); // building the state of the game in the beginning of the turn
            MySatelliteView satellite(boardView);
            Logger::debug("Board after Turn " + turn + ":");
            for (const auto& row : boardView) {
                std::string line;
                for (char cell : row) {
                    line += cell;
                }
                Logger::debug(line);
            }
            for (auto& [algoPtr, tank] : tankPairs) {
                int birthIdx = tank->getBirthIndex();
                if (! tank->isAlive()) continue;
                ActionRequest request = algoPtr->getAction();
                Logger::debug(tank->getFullIdString() + ": Requested action - " + actionToString(request));
                std::string actionStr = actionToString(request);
                if (request == ActionRequest::GetBattleInfo) {
                    tankLog[birthIdx].lastAction = "GetBattleInfo";
                    Logger::debug(tank->getFullIdString() + ": GetBattleInfo triggered");
                    satellite.setPosition(tank->getPosition());
                    if (tank->getSymbol() == '1') {
                        player1->updateTankWithBattleInfo(*algoPtr, satellite);
                    } else { player2->updateTankWithBattleInfo(*algoPtr, satellite); }
                } else {
                    executeAction(request, tank);
                    if (!tank->isAlive()) {
                        actionStr += " (killed)";
                        tankLog[birthIdx].isAlive = false;
                        tankLog[birthIdx].wasKilledThisTurn = true;
                    }
                    tankLog[birthIdx].lastAction = actionStr;
                    Logger::debug(tank->getFullIdString() + ": Action executed - " + actionStr);
                }
            }
            checkCollisions();
            if (noShellsLeftForAllLiveTanks()) {
                stepsWithoutShells++;
                Logger::debug("No shells left for all live tanks. Counter: " + std::to_string(stepsWithoutShells));
            }
        }
        currentStep++;
    }
    logGameResult();
}

// handling the backward status of the tank (if needed)
bool GameManager::handleBackwardStatus(Tank* tank, const ActionRequest& req, const Position& pos, const Direction& dir) {
    if (tank->getBackwardStatus() == 3 && req != ActionRequest::MoveForward) {
        tank->decreaseBackwardStatus();
        return true;
    }
    if (tank->getBackwardStatus() == 3 && req == ActionRequest::MoveForward) {
        tank->setBackwardStatus(0);
        return true;
    }
    if (tank->getBackwardStatus() == 2) {
        Position back = pos + dir.getOppositeDirection().toVector();
        wrapPosition(back);
        tank->moveBackward(gameBoard->getWidth(), gameBoard->getHeight());
        Logger::debug(tank->getFullIdString() + " moved backward.");
        tank->decreaseBackwardStatus();
        return true;
    }
    if (tank->getBackwardStatus() == 1 && req != ActionRequest::MoveBackward) {
        tank->decreaseBackwardStatus();
    }
    return false; // procceed as usual in executeAction
}

void GameManager::executeAction(const ActionRequest& req, Tank* tank) {
    Position pos = tank->getPosition();
    Direction dir = tank->getDirection();
    std::string player = (tank->getSymbol() == '1') ? "Player 1" : "Player 2";
    Logger::debug(tank->getFullIdString() + " initiates action:");
    if (handleBackwardStatus(tank, req, pos, dir)) { return; }
    switch (req) {
        case ActionRequest::MoveForward: {
            Position next = pos + dir.toVector();
            wrapPosition(next);
            if (gameBoard->isPassable(next.getx(), next.gety())) {
                gameBoard->moveTank(tank, next);
                // tank->moveForward(gameBoard->getWidth(), gameBoard->getHeight());
                Logger::debug(tank->getFullIdString() + ": MoveForward to (" + std::to_string(next.getx()) + ", " + std::to_string(next.gety()) + ")");
            } else {
                Logger::debug(tank->getFullIdString() + ": Bad step - blocked forward.");
                int birthIdx = tank->getBirthIndex();
                tankLog[birthIdx].lastAction += "(ignored)";
            }
            break;
        }
        case ActionRequest::MoveBackward: {
            Position back = pos + dir.getOppositeDirection().toVector();
            wrapPosition(back);
            if (tank->getBackwardStatus() == 0 && gameBoard->isPassable(back.getx(), back.gety())) {
                tank->setBackwardStatus(3);
                Logger::debug(tank->getFullIdString() + ": Started MoveBackward process.");
            }
            break;
        }
        case ActionRequest::Shoot: {
            if (tank->getRemainingShells() <= 0) {
                Logger::debug(tank->getFullIdString() + ": Bad step - no shells left.");
                int birthIdx = tank->getBirthIndex();
                tankLog[birthIdx].lastAction += "(ignored)";
                return;
            }
            tank->decreaseShells();
            Position shoot = pos + dir.toVector();
            wrapPosition(shoot);
            gameBoard->addShell(std::make_unique<Shell>(shoot, dir, player[0]));
            Logger::debug(tank->getFullIdString() + ": Shoot from (" + std::to_string(pos.getx()) + ", " + std::to_string(pos.gety()) + ") to (" + std::to_string(shoot.getx()) + ", " + std::to_string(shoot.gety()) + ")");
            break;
        }
        case ActionRequest::RotateLeft45: tank->rotateLeft8(); break;
        case ActionRequest::RotateRight45: tank->rotateRight8(); break;
        case ActionRequest::RotateLeft90: tank->rotateLeft4(); break;
        case ActionRequest::RotateRight90: tank->rotateRight4(); break;
        default:
            Logger::debug(tank->getFullIdString() + ": No action taken.");
            break;
    }
    Logger::debug(tank->getFullIdString() + ": Direction is now " + std::to_string(tank->getDirection().getDirection()));
}

void GameManager::wrapPosition(Position& pos) {
    pos.setx((pos.getx() + gameBoard->getWidth()) % gameBoard->getWidth());
    pos.sety((pos.gety() + gameBoard->getHeight()) % gameBoard->getHeight());
}

// helpfer function to build the matrix of the game board in the beginning of each turn, to give to SatelliteView
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
                    Logger::debug("Shell detected at (" + std::to_string(x) + "," + std::to_string(y) + ")");
                }
                else{board[y][x] = '@';}}
            else if (!slot.getShells().empty()){
                if (!slot.getShells().empty()) {
                    Logger::debug("Shell detected at (" + std::to_string(x) + "," + std::to_string(y) + ")");
                    board[y][x] = '*';
            }
        }
        }
    }
    return board;
}

void GameManager::checkCollisions() {
    checkShellCollisions();
    checkTankMineCollisions();
    checkTankTankCollisions();
}

// checking shell with tanks, walls and other shells
void GameManager::checkShellCollisions() {
    std::vector<std::pair<Shell*, Position>> toRemove;
    for (Shell* shell : gameBoard->getShells()) {
        Position pos = shell->getPosition();
        CellSlot& slot = gameBoard->getSlot(pos.getx(), pos.gety());
        if (Tank* tank = slot.getTank()) {
            char symbol = tank->getSymbol();
            Logger::debug("Shell:" + tank->getFullIdString() + " destroyed at (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + ")");
            tank->Hit();
            if (symbol == '1') tankIndex1--;
            else if (symbol == '2') tankIndex2--;
            int birthIdx = tank->getBirthIndex();
            tankLog[birthIdx].isAlive = false;
            tankLog[birthIdx].wasKilledThisTurn = true;
            tankPairs.erase(std::remove_if(tankPairs.begin(), tankPairs.end(), [tank](const auto& pair) { return pair.second == tank; }), tankPairs.end());
            gameBoard->removeTankAt(pos.getx(), pos.gety());
            toRemove.emplace_back(shell, pos);
            continue;
        }
        if (Wall* wall = slot.getWall()) {
            int hp = wall->onHit();
            Logger::debug("Shell hit wall at (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + "). Wall HP: " + std::to_string(hp));
            if (hp <= 0) {
                gameBoard->removeWallAt(pos.getx(), pos.gety());
                Logger::debug("Wall destroyed at (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + ")");
            }
            toRemove.emplace_back(shell, pos);
            continue;
        }
        for (Shell* other : slot.getShells()) {
            if (other != shell && other->getPosition() == pos) {
                toRemove.emplace_back(shell, pos);
                toRemove.emplace_back(other, pos);
                Logger::debug("Shells collided at (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + ")");
                break;
            }
        }
    }
    for (const auto& [s, pos] : toRemove) {
        gameBoard->removeShell(s, pos.getx(), pos.gety());
    }
}

// checking if tanks hit mines
void GameManager::checkTankMineCollisions() {
    for (auto it = tankPairs.begin(); it != tankPairs.end(); ) {
        Tank* tank = it->second;
        if (!tank->isAlive()) {
            ++it;
            continue;
        }
        Position pos = tank->getPosition();
        CellSlot& slot = gameBoard->getSlot(pos.getx(), pos.gety());
        if (slot.getMine()) {
            char symbol = tank->getSymbol(); 
            Logger::debug("Mine:" + tank->getFullIdString() + " hit a mine at (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + ")");
            tank->Hit();
            if (symbol == '1') tankIndex1--;
            else if (symbol == '2') tankIndex2--;
            int birthIdx = tank->getBirthIndex();
            tankLog[birthIdx].isAlive = false;
            tankLog[birthIdx].wasKilledThisTurn = true;
            gameBoard->removeTankAt(pos.getx(), pos.gety());
            gameBoard->removeMineAt(pos.getx(), pos.gety());
            it = tankPairs.erase(it);
        } else {
            ++it;
        }
    }
}

void GameManager::checkTankTankCollisions() {
    for (int y = 0; y < gameBoard->getHeight(); ++y) {
        for (int x = 0; x < gameBoard->getWidth(); ++x) {
            CellSlot& slot = gameBoard->getSlot(x, y);

            // שלב ראשון: אסוף את כל הפוינטרים לטנקים בתא
            std::vector<Tank*> tanks;
            for (const auto& obj : slot.getAll()) {
                if (Tank* tank = dynamic_cast<Tank*>(obj.get()))
                    tanks.push_back(tank);
            }

            if (tanks.size() > 1) {
                std::string posStr = "(" + std::to_string(x) + "," + std::to_string(y) + ")";
                Logger::debug("Tank-Tank collision at " + posStr);

                // לא מוחקים מתוך הלולאה המקורית! קודם אוספים ואז מוחקים
                std::vector<Tank*> toRemove = tanks;

                for (Tank* tank : toRemove) {
                    char symbol = tank->getSymbol();
                    tank->Hit();
                    if (symbol == '1') tankIndex1--;
                    else if (symbol == '2') tankIndex2--;
                    int birthIdx = tank->getBirthIndex();
                    tankLog[birthIdx].isAlive = false;
                    tankLog[birthIdx].wasKilledThisTurn = true;
                    tankPairs.erase(
                        std::remove_if(tankPairs.begin(), tankPairs.end(),
                            [tank](const auto& pair) { return pair.second == tank; }),
                        tankPairs.end());
                    slot.removeTank(tank);
                }
            }
        }
    }
}




// checking if there are no shells left for all live tanks, to start the counter of turns
bool GameManager::noShellsLeftForAllLiveTanks() const {
    for (const auto& [algo, tank] : tankPairs) {
        if (tank->isAlive() && tank->getRemainingShells() > 0)
            return false;
    }
    return true;
}

bool GameManager::checkGameOver() const {
    return tankIndex1==0 || tankIndex2==0|| currentStep/2 >= maxSteps || stepsWithoutShells > NO_SHELL_LIMIT;
}

void GameManager::logGameResult() {
    Logger::debug("Game Over!");

    if (tankIndex1 == 0 && tankIndex2 == 0)
        Logger::debug("Tie - Both players lost all tanks");
    else if (tankIndex1 == 0)
        Logger::debug("Player 2 wins - Player 1 eliminated");
    else if (tankIndex2 == 0)
        Logger::debug("Player 1 wins - Player 2 eliminated");
    else
        Logger::debug("Tie - Reached max steps or no shells left for all live tanks");
    int alive1 = 0, alive2 = 0;
    for (const auto& t : tankLog) {
        if (t.isAlive) {
            if (t.symbol == '1') ++alive1;
            else if (t.symbol == '2') ++alive2;
        }
    }
    if (alive1 == 0 && alive2 == 0) {
        simpleOutput << "Tie, both players have zero tanks" << std::endl;
    } else if (currentStep/2 >= maxSteps) {
        simpleOutput << "Tie, reached max steps = " << maxSteps << ", player 1 has " << alive1 << " tanks, player 2 has " << alive2 << " tanks" << std::endl;
    } else if (stepsWithoutShells > NO_SHELL_LIMIT) {
        simpleOutput << "Tie, both players have zero shells for " << NO_SHELL_LIMIT << " steps" << std::endl;
    } else if (alive1 > 0) {
        simpleOutput << "Player 1 won with " << alive1 << " tanks still alive" << std::endl;
    } else {
        simpleOutput << "Player 2 won with " << alive2 << " tanks still alive" << std::endl;
    }
}