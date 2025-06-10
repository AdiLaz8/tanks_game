#include "GameManager.h"
#include "Shell.h"
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
void GameManager::readBoardHeader(const std::string& filename, std::ifstream& file, size_t& rows, size_t& cols) {
    file.open(filename);
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
                    t->setRemainingShells(numShells);
                    auto raw = dynamic_cast<TankAlgorithm*>(base.get());
                    if (t && raw) {
                        std::unique_ptr<TankAlgorithm> algo(static_cast<TankAlgorithm*>(base.release()));
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
                    gameBoard->addObject(std::make_unique<Tank>('2', Direction(Direction::R), Position(x, y)), x, y);
                    hasTank2 = true;
                    std::unique_ptr<TankAlgorithm> base = tankAlgoFactory.create(2, tankIndex2);
                    Tank* t = gameBoard->getSlot(x, y).getTank();
                    t->setRemainingShells(numShells);
                    auto raw = dynamic_cast<TankAlgorithm*>(base.get());
                    if (t && raw) {
                        std::unique_ptr<TankAlgorithm> algo(dynamic_cast<TankAlgorithm*>(base.release()));
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
    populateTankOrderAndLog(rows, cols);
    finalizeBoardReading(file, errorFile, hasTank1, hasTank2);
}

void GameManager::populateTankOrderAndLog(size_t rows, size_t cols) {
    int birthIdx = 0;
    for (size_t y = 0; y < rows; ++y) {
        for (size_t x = 0; x < cols; ++x) {
            Tank* t = gameBoard->getSlot(x, y).getTank();
            if (t) {
                t->birthIndex = birthIdx++;
                tanksOrderedByBirth.push_back(t);
            }
        }
    }

    for (Tank* t : tanksOrderedByBirth) {
        TankLogInfo info;
        info.symbol = t->getSymbol();
        tankLog.push_back(info);
    }
}

void GameManager::finalizeBoardReading(std::ifstream& file, std::ofstream& errorFile, bool hasTank1, bool hasTank2) {
    // פתיחת קובץ הפלט הפשוט
    simpleOutput.open("output.txt");
    if (!simpleOutput.is_open()) {
        std::cerr << "Error: Cannot open simple output file!" << std::endl;
        exit(1);
    }

    // קריאת שורות מיותרות (אם יש)
    std::string extra;
    while (std::getline(file, extra)) {
        if (!extra.empty()) errorFile << "Warning: Extra row ignored.\n";
    }
    errorFile.close();

    // בדיקות תקפות
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
    std::vector<Shell*> copy = gameBoard->getShells(); // לא נוגעים ב־ownedShells

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
                if (!tankLog[i].isAlive && tankLog[i].wasKilledThisTurn) {
                    action = tankLog[i].lastAction + " (killed)";
                } else if (!tankLog[i].isAlive) {
                    action = "killed";
                } else {
                    action = tankLog[i].lastAction;
                }
                simpleOutput << action;
                if (i + 1 < tankLog.size()) simpleOutput << ", ";
            }
            simpleOutput << std::endl;
            for (auto& info : tankLog) {
                info.wasKilledThisTurn = false;
            }
        } else {
            std::string turn = std::to_string(currentStep / 2 + 1);
            Logger::debug("Turn : " + turn);
            logFile << "Turn : " + turn << std::endl;
            // אפס את currentActions לכל הטנקים לפי birthIndex
            moveShells();
            checkCollisions();
            if (checkGameOver()) break;
            auto boardView = buildBoardMatrix();
            MySatelliteView satellite(boardView);
            logFile << "Board after Turn " << (currentStep / 2 + 1) << ":\n";
            for (const auto& row : boardView) {
                for (char cell : row) {
                    logFile << cell;
                }
                logFile << '\n';
}
            for (auto& [algoPtr, tank] : tankPairs) {
                int birthIdx = tank->birthIndex;
                if (! tank->isAlive()) continue;
                ActionRequest request = algoPtr->getAction();
                logFile << "Player " << tank->getSymbol() << ": Requested action - " << static_cast<int>(request) << std::endl;
                std::string actionStr = actionToString(request);
                if (request == ActionRequest::GetBattleInfo) {
                    tankLog[birthIdx].lastAction = "GetBattleInfo";
                    logFile << "Player " << tank->getSymbol() << ": GetBattleInfo triggered\n";
                    satellite.setPosition(tank->getPosition());
                    if (tank->getSymbol() == '1') {
                        player1->updateTankWithBattleInfo(*algoPtr, satellite);
                    } else {
                        player2->updateTankWithBattleInfo(*algoPtr, satellite);
                    }
                } else {
                    executeAction(request, tank);
                    // תעד פעולה
                    if (!tank->isAlive()) {
                        actionStr += " (killed)";
                        tankLog[birthIdx].isAlive = false;
                        tankLog[birthIdx].wasKilledThisTurn = true;
                    }
                    tankLog[birthIdx].lastAction = actionStr;
                    logFile << "Player " << tank->getSymbol() << ": doing action - " << static_cast<int>(request) << std::endl;
                }
            }
            checkCollisions();
            if (noShellsLeftForAllLiveTanks()) {
                stepsWithoutShells++;
                logFile << "No shells left for all live tanks. Counter: " << stepsWithoutShells << std::endl;
            } else {
                stepsWithoutShells = 0;
            }
        }
        currentStep++;
    }
    logGameResult();
}


void GameManager::executeAction(const ActionRequest& req, Tank* tank) {
    Position pos = tank->getPosition();
    Direction dir = tank->getDirection();
    std::string player = (tank->getSymbol() == '1') ? "Player 1" : "Player 2";
    Logger::debug(player + " initiates action:");
    if (tank->getBackwardStatus() == 3 && req != ActionRequest::MoveForward) {
        tank->decreaseBackwardStatus();
        return;
    }
    if (tank->getBackwardStatus() == 3 && req == ActionRequest::MoveForward) {
        tank->setBackwardStatus(0);
        return;
    }
    if (tank->getBackwardStatus() == 2) {
        Position back = pos + dir.getOppositeDirection().toVector();
        wrapPosition(back);
        tank->moveBackward(gameBoard->getWidth(), gameBoard->getHeight());
        logFile << player << ": Moving backward now." << std::endl;
        Logger::debug(player + " moved backward.");
        tank->decreaseBackwardStatus();
        return;
    }
    if (tank->getBackwardStatus() == 1 && req != ActionRequest::MoveBackward) {
        tank->decreaseBackwardStatus();
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
                int birthIdx = tank->birthIndex;
                tankLog[birthIdx].lastAction += "(ignored)";
            }
            break;
        }
        case ActionRequest::MoveBackward: {
            Position back = pos + dir.getOppositeDirection().toVector();
            wrapPosition(back);
            if (tank->getBackwardStatus() == 0 && gameBoard->isPassable(back.getx(), back.gety())) {
                tank->setBackwardStatus(3);
                logFile << player << ": Started MoveBackward process." << std::endl;
            }
            break;
        }
        case ActionRequest::Shoot: {
            Position shoot = pos + dir.toVector();
            wrapPosition(shoot);
            gameBoard->addShell(std::make_unique<Shell>(shoot, dir, player[0]));
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
                    logFile << "Shell detected at (" << x << "," << y << ")" << std::endl;
                }
                else{board[y][x] = '@';}}
            else if (!slot.getShells().empty()){
                if (!slot.getShells().empty()) {
                    logFile << "Shell detected at (" << x << "," << y << ")" << std::endl;
                    board[y][x] = '*';
            }
        }
        }
    }
    return board;
}

void GameManager::checkCollisions() {
    std::vector<std::pair<Shell*, Position>> toRemove;
    // מעבר על כל הפגזים ובדיקת התנגשויות
    for (Shell* shell : gameBoard->getShells()) {
        Position pos = shell->getPosition();
        CellSlot& slot = gameBoard->getSlot(pos.getx(), pos.gety());
        if (Tank* tank = slot.getTank()) {
            std::cout << "[COLLISION] Shell at (" << pos.getx() << "," << pos.gety()
              << ") hit TANK of player " << tank->getSymbol()
              << " at (" << tank->getPosition().getx() << "," << tank->getPosition().gety() << ")"
              << ", shell owner: player " << shell->getOwnerId() << std::endl;
            char symbol = tank->getSymbol();  // שמור מראש לפני מחיקה
            tank->Hit();
            if (symbol == '1') tankMap1--;
            else if (symbol == '2') tankMap2--;
            int birthIdx = tank->birthIndex;
            tankLog[birthIdx].isAlive = false;
            tankLog[birthIdx].wasKilledThisTurn = true;

            tankPairs.erase(std::remove_if(tankPairs.begin(), tankPairs.end(),
                [tank](const auto& pair) { return pair.second == tank; }), tankPairs.end());

            gameBoard->removeTankAt(pos.getx(), pos.gety());
            logFile << "Shell: Tank " << symbol << " destroyed at (" << pos.getx() << "," << pos.gety() << ")" << std::endl;
            Logger::debug("Shell destroyed tank at position (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + ")");
            toRemove.emplace_back(shell, pos);
            continue;
        }

        if (Wall* wall = slot.getWall()) {
            int hp = wall->onHit();
            logFile << "Shell hit wall at (" << pos.getx() << "," << pos.gety() << "). Wall HP: " << hp << std::endl;
            if (hp <= 0) {
                gameBoard->removeWallAt(pos.getx(), pos.gety());
                logFile << "Wall destroyed at (" << pos.getx() << "," << pos.gety() << ")" << std::endl;
            }
            toRemove.emplace_back(shell, pos);
            continue;
        }

        for (Shell* other : slot.getShells()) {
            if (other != shell && other->getPosition() == pos) {
                toRemove.emplace_back(shell, pos);
                toRemove.emplace_back(other, pos);
                logFile << "Shells collided at (" << pos.getx() << "," << pos.gety() << ")" << std::endl;
                break;
            }
        }
    }

    // מחיקת הפגזים שנפגעו
    for (const auto& [s, pos] : toRemove) {
        gameBoard->removeShell(s, pos.getx(), pos.gety());
    }

    // מעבר על הטנקים ובדיקת מוקשים
    for (auto it = tankPairs.begin(); it != tankPairs.end(); ) {
        Tank* tank = it->second;
        if (!tank->isAlive()) {
            ++it;
            continue;
        }

        Position pos = tank->getPosition();
        CellSlot& slot = gameBoard->getSlot(pos.getx(), pos.gety());

        if (slot.getMine()) {
            char symbol = tank->getSymbol();  // שמור מראש
            tank->Hit();
            if (symbol == '1') tankMap1--;
            else if (symbol == '2') tankMap2--;
            int birthIdx = tank->birthIndex;
            tankLog[birthIdx].isAlive = false;
            tankLog[birthIdx].wasKilledThisTurn = true;
            gameBoard->removeTankAt(pos.getx(), pos.gety());
            gameBoard->removeMineAt(pos.getx(), pos.gety());

            logFile << "Mine: Player " << symbol << " tank hit a mine at ("
                    << pos.getx() << "," << pos.gety() << ")" << std::endl;
            it = tankPairs.erase(it);
        } else {
            ++it;
        }
    }
}

bool GameManager::noShellsLeftForAllLiveTanks() const {
    for (const auto& [algo, tank] : tankPairs) {
        if (tank->isAlive() && tank->getRemainingShells() > 0)
            return false;
    }
    return true;
}

bool GameManager::checkGameOver() const {
    return tankMap1==0 || tankMap2==0|| currentStep/2 >= maxSteps || stepsWithoutShells >= NO_SHELL_LIMIT;
}

void GameManager::logGameResult() {
    logFile << "Game Over!" << std::endl;
    Logger::debug("Game Over!");

    // שמירה על ההדפסות הקיימות ל־logFile
    if (tankMap1 == 0 && tankMap2 == 0)
        logFile << "RESULT: Tie - Both players lost all tanks" << std::endl;
    else if (tankMap1 == 0)
        logFile << "RESULT: Player 2 wins - Player 1 eliminated" << std::endl;
    else if (tankMap2 == 0)
        logFile << "RESULT: Player 1 wins - Player 2 eliminated" << std::endl;
    else
        logFile << "RESULT: Tie - Reached max steps" << std::endl;

    // === חלק חדש: סיכום ל־simpleOutput ===
    int alive1 = 0, alive2 = 0;
    for (const auto& t : tankLog) {
        if (t.isAlive) {
            if (t.symbol == '1') ++alive1;
            else if (t.symbol == '2') ++alive2;
        }
    }
    // הגדרת תנאים בדיוק כפי שביקשת
    if (alive1 == 0 && alive2 == 0) {
        simpleOutput << "Tie, both players have zero tanks" << std::endl;
    } else if (currentStep/2 >= maxSteps) {
        simpleOutput << "Tie, reached max steps = " << maxSteps << ", player 1 has " << alive1 << " tanks, player 2 has " << alive2 << " tanks" << std::endl;
    } else if (stepsWithoutShells >= NO_SHELL_LIMIT) {
        simpleOutput << "Tie, both players have zero shells for " << NO_SHELL_LIMIT << " steps" << std::endl;
    } else if (alive1 > 0) {
        simpleOutput << "Player 1 won with " << alive1 << " tanks still alive" << std::endl;
    } else {
        simpleOutput << "Player 2 won with " << alive2 << " tanks still alive" << std::endl;
    }
    simpleOutput.close();
}