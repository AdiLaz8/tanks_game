#include "GameManager_318772340_206580102.h"
#include "Shell.h"
#include "Wall.h"
#include "Mine.h"
#include "../UserCommon/Logger.h"
#include "ActionRequestUtils.h"
#include "CellSlot.h"
#include "../common/GameManagerRegistration.h"
#include <iostream>
#include <unordered_set>

// Use UserCommon namespace
using namespace UserCommon_318772340_206580102;

namespace GameManager_318772340_206580102 {

GameManager_318772340_206580102::GameManager_318772340_206580102(bool verbose) : verbose(verbose), currentStep(0) {}

GameManager_318772340_206580102::~GameManager_318772340_206580102() {
    if (simpleOutput.is_open()) {
        simpleOutput.close();
    }
}

GameResult GameManager_318772340_206580102::run(
    size_t map_width, size_t map_height,
    const SatelliteView& map,
    string map_name,
    size_t max_steps, size_t num_shells,
    Player& player1, string /*name1*/, Player& player2, string /*name2*/, 
    TankAlgorithmFactory player1_tank_algo_factory,
    TankAlgorithmFactory player2_tank_algo_factory) {
    
    std::cout << "DEBUG: GameManager::run() started" << std::endl;
    
    // Initialize game state
    maxSteps = max_steps;
    numShells = num_shells;
    currentStep = 0;
    stepsWithoutShells = 0;
    tankIndex1 = 0;
    tankIndex2 = 0;
    
    std::cout << "DEBUG: Game state initialized: maxSteps=" << maxSteps << ", numShells=" << numShells << std::endl;
    
    // Create game board
    gameBoard = std::make_unique<Board>(map_width, map_height);
    std::cout << "DEBUG: Game board created" << std::endl;
    
    // Parse map and create tanks with algorithms
    std::cout << "DEBUG: About to parse map..." << std::endl;
    parseMapFromSatelliteView(map, map_width, map_height, player1, player2,
                             player1_tank_algo_factory, player2_tank_algo_factory);
    std::cout << "DEBUG: Map parsed, tankIndex1=" << tankIndex1 << ", tankIndex2=" << tankIndex2 << std::endl;
    
    // Setup tank logging
    std::cout << "DEBUG: About to populate tank order and log..." << std::endl;
    populateTankOrderAndLog(map_height, map_width);
    std::cout << "DEBUG: Tank order and log populated" << std::endl;
    
    // Check if valid game setup
    if (tankIndex1 == 0 && tankIndex2 == 0) {
        std::cout << "DEBUG: No tanks found, returning early" << std::endl;
        GameResult result;
        result.winner = 0;
        result.reason = GameResult::ALL_TANKS_DEAD;
        result.remaining_tanks = {0, 0};
        result.rounds = 0;
        result.gameState = std::make_unique<MySatelliteView>(buildBoardMatrix());
        return result;
    }
    
    // Open output file if verbose mode
    if (verbose) {
        std::string outputFile = "output_" + map_name;
        simpleOutput.open(outputFile);
        std::cout << "DEBUG: Output file opened: " << outputFile << std::endl;
    }
    
    // Execute game loop
    std::cout << "DEBUG: About to execute game loop..." << std::endl;
    return executeGameLoop(player1, player2);
}

void GameManager_318772340_206580102::parseMapFromSatelliteView(const SatelliteView& map, size_t width, size_t height,
                                           Player& /*player1*/, Player& /*player2*/,
                                           TankAlgorithmFactory factory1, TankAlgorithmFactory factory2) {
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            char c = map.getObjectAt(x, y);
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
                    TankAlgorithmFactory factory = (c == '1') ? factory1 : factory2;
                    
                    gameBoard->addObject(std::make_unique<Tank>(c, dir, Position(x, y)), x, y);
                    std::unique_ptr<TankAlgorithm> algo = factory(playerId, tankIdx);
                    Tank* t = gameBoard->getSlot(x, y).getTank();
                    t->setTankIndex(tankIdx);
                    t->setRemainingShells(numShells);
                    tankPairs.emplace_back(std::move(algo), t);
                    
                    if (verbose) {
                        Logger::debug(t->getFullIdString() + " tank placed at (" + std::to_string(x) + "," + std::to_string(y) + ")");
                    }
                    tankIdx++;
                    break;
                }
                default:
                    if (verbose) {
                        Logger::debug("Warning: Unknown char '" + std::string(1, c) + "' at (" + std::to_string(x) + "," + std::to_string(y) + ")");
                    }
            }
        }
    }
}

GameResult GameManager_318772340_206580102::executeGameLoop(Player& player1, Player& player2) {
    std::cout << "DEBUG: executeGameLoop started" << std::endl;
    std::cout << "DEBUG: Initial checkGameOver() = " << (checkGameOver() ? "true" : "false") << std::endl;
    std::cout << "DEBUG: tankIndex1=" << tankIndex1 << ", tankIndex2=" << tankIndex2 << ", currentStep=" << currentStep << ", maxSteps=" << maxSteps << std::endl;
    
    while (!checkGameOver()) {
        std::cout << "DEBUG: Game loop iteration, currentStep=" << currentStep << std::endl;
        if (currentStep % 2 != 0) {
            std::cout << "DEBUG: Odd step (shell movement phase), currentStep=" << currentStep << std::endl;
            moveShells();
            checkCollisions();
            
            if (verbose && simpleOutput.is_open()) {
                for (size_t i = 0; i < tankLog.size(); ++i) {
                    std::string action;
                    if (!tankLog[i].isAlive && tankLog[i].wasKilledThisTurn) { 
                        action = tankLog[i].lastAction + " (killed)"; 
                    }
                    else if (!tankLog[i].isAlive) { 
                        action = "killed"; 
                    }
                    else { 
                        action = tankLog[i].lastAction; 
                    }
                    simpleOutput << action;
                    if (i + 1 < tankLog.size()) simpleOutput << ", ";
                }
                simpleOutput << std::endl;
                for (auto& info : tankLog) { info.wasKilledThisTurn = false; }
            }
        } else {
            std::cout << "DEBUG: Even step (tank action phase), currentStep=" << currentStep << std::endl;
            std::string turn = std::to_string(currentStep / 2 + 1);
            std::cout << "DEBUG: Turn " << turn << " starting" << std::endl;
            if (verbose) {
                Logger::debug("Turn : " + turn);
            }
            
            std::cout << "DEBUG: Moving shells..." << std::endl;
            moveShells();
            std::cout << "DEBUG: Checking collisions..." << std::endl;
            checkCollisions();
            std::cout << "DEBUG: Checking game over after shell movement..." << std::endl;
            if (checkGameOver()) {
                std::cout << "DEBUG: Game over detected, breaking loop" << std::endl;
                break;
            }
            
            std::cout << "DEBUG: Building board matrix..." << std::endl;
            auto boardView = buildBoardMatrix();
            std::cout << "DEBUG: Creating satellite view..." << std::endl;
            MySatelliteView satellite(boardView);
            
            if (verbose) {
                Logger::debug("Board after Turn " + turn + ":");
                for (const auto& row : boardView) {
                    std::string line;
                    for (char cell : row) {
                        line += cell;
                    }
                    Logger::debug(line);
                }
            }
            
            std::cout << "DEBUG: Starting tank action loop, tankPairs.size()=" << tankPairs.size() << std::endl;
            for (auto& [algoPtr, tank] : tankPairs) {
                int birthIdx = tank->getBirthIndex();
                std::cout << "DEBUG: Processing tank " << tank->getFullIdString() << ", birthIdx=" << birthIdx << ", isAlive=" << tank->isAlive() << std::endl;
                if (!tank->isAlive()) {
                    std::cout << "DEBUG: Tank " << tank->getFullIdString() << " is dead, skipping" << std::endl;
                    continue;
                }
                
                std::cout << "DEBUG: About to get action from " << tank->getFullIdString() << std::endl;
                ActionRequest request = algoPtr->getAction();
                std::cout << "DEBUG: " << tank->getFullIdString() << " requested action: " << actionToString(request) << std::endl;
                if (verbose) {
                    Logger::debug(tank->getFullIdString() + ": Requested action - " + actionToString(request));
                }
                
                std::string actionStr = actionToString(request);
                if (request == ActionRequest::GetBattleInfo) {
                    std::cout << "DEBUG: " << tank->getFullIdString() << " requested GetBattleInfo" << std::endl;
                    tankLog[birthIdx].lastAction = "GetBattleInfo";
                    if (verbose) {
                        Logger::debug(tank->getFullIdString() + ": GetBattleInfo triggered");
                    }
                    satellite.setPosition(tank->getPosition());
                    if (tank->getSymbol() == '1') {
                        std::cout << "DEBUG: Updating Player 1 tank with battle info" << std::endl;
                        player1.updateTankWithBattleInfo(*algoPtr, satellite);
                    } else { 
                        std::cout << "DEBUG: Updating Player 2 tank with battle info" << std::endl;
                        player2.updateTankWithBattleInfo(*algoPtr, satellite); 
                    }
                } else {
                    std::cout << "DEBUG: Executing action " << actionStr << " for " << tank->getFullIdString() << std::endl;
                    executeAction(request, tank);
                    if (!tank->isAlive()) {
                        std::cout << "DEBUG: Tank " << tank->getFullIdString() << " was killed" << std::endl;
                        actionStr += " (killed)";
                        tankLog[birthIdx].isAlive = false;
                        tankLog[birthIdx].wasKilledThisTurn = true;
                    }
                    tankLog[birthIdx].lastAction = actionStr;
                    if (verbose) {
                        Logger::debug(tank->getFullIdString() + ": Action executed - " + actionStr);
                    }
                }
            }
            
            std::cout << "DEBUG: Tank action loop completed" << std::endl;
            std::cout << "DEBUG: Checking collisions after tank actions..." << std::endl;
            checkCollisions();
            std::cout << "DEBUG: Checking if no shells left for all live tanks..." << std::endl;
            if (noShellsLeftForAllLiveTanks()) {
                stepsWithoutShells++;
                std::cout << "DEBUG: No shells left, stepsWithoutShells=" << stepsWithoutShells << std::endl;
                if (verbose) {
                    Logger::debug("No shells left for all live tanks. Counter: " + std::to_string(stepsWithoutShells));
                }
            }
        }
        std::cout << "DEBUG: About to increment currentStep from " << currentStep << " to " << (currentStep + 1) << std::endl;
        currentStep++;
        std::cout << "DEBUG: currentStep incremented to " << currentStep << std::endl;
    }
    
    if (verbose) {
        logGameResult();
    }
    
    // Create GameResult
    GameResult result;
    int alive1 = 0, alive2 = 0;
    for (const auto& t : tankLog) {
        if (t.isAlive) {
            if (t.symbol == '1') ++alive1;
            else if (t.symbol == '2') ++alive2;
        }
    }
    
    result.remaining_tanks = {static_cast<size_t>(alive1), static_cast<size_t>(alive2)};
    result.rounds = currentStep / 2;
    result.gameState = std::make_unique<MySatelliteView>(buildBoardMatrix());
    
    if (tankIndex1 == 0 && tankIndex2 == 0) {
        result.winner = 0;
        result.reason = GameResult::ALL_TANKS_DEAD;
    } else if (tankIndex1 == 0) {
        result.winner = 2;
        result.reason = GameResult::ALL_TANKS_DEAD;
    } else if (tankIndex2 == 0) {
        result.winner = 1;
        result.reason = GameResult::ALL_TANKS_DEAD;
    } else if (currentStep/2 >= maxSteps) {
        result.winner = 0;
        result.reason = GameResult::MAX_STEPS;
    } else {
        result.winner = 0;
        result.reason = GameResult::ZERO_SHELLS;
    }
    
    return result;
}

// Existing methods adapted with namespace and verbose logging...
void GameManager_318772340_206580102::moveShells() {
    std::vector<Shell*> copy = gameBoard->getShells(); 
    for (Shell* shell : copy) {
        Position oldPos = shell->getPosition();
        Position newPos = oldPos + shell->getDirection().toVector();
        wrapPosition(newPos);
        gameBoard->moveShellTo(shell, oldPos.getx(), oldPos.gety(), newPos.getx(), newPos.gety());
        shell->setPosition(newPos);
    }
}

bool GameManager_318772340_206580102::handleBackwardStatus(Tank* tank, const ActionRequest& req, const Position& pos, const Direction& dir) {
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
        if (verbose) {
            Logger::debug(tank->getFullIdString() + " moved backward.");
        }
        tank->decreaseBackwardStatus();
        return true;
    }
    if (tank->getBackwardStatus() == 1 && req != ActionRequest::MoveBackward) {
        tank->decreaseBackwardStatus();
    }
    return false;
}

void GameManager_318772340_206580102::executeAction(const ActionRequest& req, Tank* tank) {
    Position pos = tank->getPosition();
    Direction dir = tank->getDirection();
    std::string player = (tank->getSymbol() == '1') ? "Player 1" : "Player 2";
    if (verbose) {
        Logger::debug(tank->getFullIdString() + " initiates action:");
    }
    if (handleBackwardStatus(tank, req, pos, dir)) { return; }
    
    switch (req) {
        case ActionRequest::MoveForward: {
            Position next = pos + dir.toVector();
            wrapPosition(next);
            if (gameBoard->isPassable(next.getx(), next.gety())) {
                gameBoard->moveTank(tank, next);
                if (verbose) {
                    Logger::debug(tank->getFullIdString() + ": MoveForward to (" + std::to_string(next.getx()) + ", " + std::to_string(next.gety()) + ")");
                }
            } else {
                if (verbose) {
                    Logger::debug(tank->getFullIdString() + ": Bad step - blocked forward.");
                }
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
                if (verbose) {
                    Logger::debug(tank->getFullIdString() + ": Started MoveBackward process.");
                }
            }
            break;
        }
        case ActionRequest::Shoot: {
            if (tank->getRemainingShells() <= 0) {
                if (verbose) {
                    Logger::debug(tank->getFullIdString() + ": Bad step - no shells left.");
                }
                int birthIdx = tank->getBirthIndex();
                tankLog[birthIdx].lastAction += "(ignored)";
                return;
            }
            tank->decreaseShells();
            Position shoot = pos + dir.toVector();
            wrapPosition(shoot);
            gameBoard->addShell(std::make_unique<Shell>(shoot, dir, player[0]));
            if (verbose) {
                Logger::debug(tank->getFullIdString() + ": Shoot from (" + std::to_string(pos.getx()) + ", " + std::to_string(pos.gety()) + ") to (" + std::to_string(shoot.getx()) + ", " + std::to_string(shoot.gety()) + ")");
            }
            break;
        }
        case ActionRequest::RotateLeft45: 
            tank->rotateLeft8(); 
            break;
        case ActionRequest::RotateRight45: 
            tank->rotateRight8(); 
            break;
        case ActionRequest::RotateLeft90: 
            tank->rotateLeft4(); 
            break;
        case ActionRequest::RotateRight90: 
            tank->rotateRight4(); 
            break;
        default:
            if (verbose) {
                Logger::debug(tank->getFullIdString() + ": No action taken.");
            }
            break;
    }
    if (verbose) {
        Logger::debug(tank->getFullIdString() + ": Direction is now " + std::to_string(tank->getDirection().getDirection()));
    }
}

void GameManager_318772340_206580102::checkCollisions() {
    std::cout << "DEBUG: Starting checkCollisions()" << std::endl;
    std::cout << "DEBUG: Calling checkShellCollisions()" << std::endl;
    checkShellCollisions();
    std::cout << "DEBUG: Calling checkTankMineCollisions()" << std::endl;
    checkTankMineCollisions();
    std::cout << "DEBUG: Calling checkTankTankCollisions()" << std::endl;
    checkTankTankCollisions();
    std::cout << "DEBUG: Finished checkCollisions()" << std::endl;
}

void GameManager_318772340_206580102::checkShellCollisions() {
    std::vector<std::pair<Shell*, Position>> toRemove;
    
    for (Shell* shell : gameBoard->getShells()) {
        Position pos = shell->getPosition();
        CellSlot& slot = gameBoard->getSlot(pos.getx(), pos.gety());
        
        if (Tank* tank = slot.getTank()) {
            char symbol = tank->getSymbol();
            if (verbose) {
                Logger::debug("Shell:" + tank->getFullIdString() + " destroyed at (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + ")");
            }
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
            if (verbose) {
                Logger::debug("Shell hit wall at (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + "). Wall HP: " + std::to_string(hp));
            }
            if (hp <= 0) {
                gameBoard->removeWallAt(pos.getx(), pos.gety());
                if (verbose) {
                    Logger::debug("Wall destroyed at (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + ")");
                }
            }
            toRemove.emplace_back(shell, pos);
            continue;
        }
        
        for (Shell* other : slot.getShells()) {
            if (other != shell && other->getPosition() == pos) {
                toRemove.emplace_back(shell, pos);
                toRemove.emplace_back(other, pos);
                if (verbose) {
                    Logger::debug("Shells collided at (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + ")");
                }
                break;
            }
        }
    }
    
    for (const auto& [s, pos] : toRemove) {
        gameBoard->removeShell(s, pos.getx(), pos.gety());
    }
}

void GameManager_318772340_206580102::checkTankMineCollisions() {
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
            if (verbose) {
                Logger::debug("Mine:" + tank->getFullIdString() + " hit a mine at (" + std::to_string(pos.getx()) + "," + std::to_string(pos.gety()) + ")");
            }
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

void GameManager_318772340_206580102::checkTankTankCollisions() {
    std::cout << "DEBUG: checkTankTankCollisions() started" << std::endl;
    for (int y = 0; y < gameBoard->getHeight(); ++y) {
        for (int x = 0; x < gameBoard->getWidth(); ++x) {
            std::cout << "DEBUG: Checking position (" << x << "," << y << ")" << std::endl;
            CellSlot& slot = gameBoard->getSlot(x, y);
            std::cout << "DEBUG: Got slot, about to call getAll()" << std::endl;
            std::vector<Tank*> tanks;
            const auto& allObjects = slot.getAll();
            std::cout << "DEBUG: getAll() returned, objects.size()=" << allObjects.size() << std::endl;
            for (const auto& obj : allObjects) {
                std::cout << "DEBUG: About to dynamic_cast object" << std::endl;
                if (Tank* tank = dynamic_cast<Tank*>(obj.get())) {
                    std::cout << "DEBUG: Found tank at (" << x << "," << y << ")" << std::endl;
                    tanks.push_back(tank);
                }
            }

            if (tanks.size() > 1) {
                if (verbose) {
                    std::string posStr = "(" + std::to_string(x) + "," + std::to_string(y) + ")";
                    Logger::debug("Tank-Tank collision at " + posStr);
                }
                std::cout << "DEBUG: Found " << tanks.size() << " tanks at position (" << x << "," << y << ")" << std::endl;
                
                // Collect all tank information before removing any tanks
                std::vector<std::tuple<Tank*, char, int, int, int>> tankInfo;
                for (Tank* tank : tanks) {
                    std::cout << "DEBUG: Collecting info for tank at (" << tank->getPosition().getx() << "," << tank->getPosition().gety() << ")" << std::endl;
                    char symbol = tank->getSymbol();
                    int birthIdx = tank->getBirthIndex();
                    int tankX = tank->getPosition().getx();
                    int tankY = tank->getPosition().gety();
                    tankInfo.push_back(std::make_tuple(tank, symbol, birthIdx, tankX, tankY));
                }
                
                // Now process all tanks using the collected information
                for (const auto& info : tankInfo) {
                    Tank* tank = std::get<0>(info);
                    char symbol = std::get<1>(info);
                    int birthIdx = std::get<2>(info);
                    int tankX = std::get<3>(info);
                    int tankY = std::get<4>(info);
                    
                    std::cout << "DEBUG: Processing tank collision for tank at (" << tankX << "," << tankY << ")" << std::endl;
                    tank->Hit();
                    if (symbol == '1') tankIndex1--;
                    else if (symbol == '2') tankIndex2--;
                    tankLog[birthIdx].isAlive = false;
                    tankLog[birthIdx].wasKilledThisTurn = true;
                    std::cout << "DEBUG: About to remove tank from tankPairs" << std::endl;
                    tankPairs.erase(
                        std::remove_if(tankPairs.begin(), tankPairs.end(),
                            [tank](const auto& pair) { return pair.second == tank; }),
                        tankPairs.end());
                    std::cout << "DEBUG: About to remove tank from board" << std::endl;
                    gameBoard->removeTankAt(tankX, tankY);
                    std::cout << "DEBUG: Tank removal completed" << std::endl;
                }
            }
        }
    }
}

bool GameManager_318772340_206580102::noShellsLeftForAllLiveTanks() const {
    for (const auto& [algo, tank] : tankPairs) {
        if (tank->isAlive() && tank->getRemainingShells() > 0)
            return false;
    }
    return true;
}

bool GameManager_318772340_206580102::checkGameOver() const {
    bool result = tankIndex1==0 || tankIndex2==0|| currentStep/2 >= maxSteps || stepsWithoutShells > NO_SHELL_LIMIT;
    std::cout << "DEBUG: checkGameOver() called: tankIndex1=" << tankIndex1 << ", tankIndex2=" << tankIndex2 
              << ", currentStep=" << currentStep << ", currentStep/2=" << (currentStep/2) << ", maxSteps=" << maxSteps 
              << ", stepsWithoutShells=" << stepsWithoutShells << ", NO_SHELL_LIMIT=" << NO_SHELL_LIMIT 
              << ", result=" << (result ? "true" : "false") << std::endl;
    return result;
}

void GameManager_318772340_206580102::wrapPosition(Position& pos) {
    pos.setx((pos.getx() + gameBoard->getWidth()) % gameBoard->getWidth());
    pos.sety((pos.gety() + gameBoard->getHeight()) % gameBoard->getHeight());
}

std::vector<std::vector<char>> GameManager_318772340_206580102::buildBoardMatrix() {
    size_t h = gameBoard->getHeight(), w = gameBoard->getWidth();
    std::vector<std::vector<char>> board(h, std::vector<char>(w, ' '));
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x) {
            CellSlot& slot = gameBoard->getSlot(x, y);
            if (slot.getTank()) board[y][x] = slot.getTank()->getSymbol();
            else if (slot.getWall()) board[y][x] = '#';
            else if (slot.getMine()) {
                if(!slot.getShells().empty()){
                    board[y][x] = '*';
                    if (verbose) {
                        Logger::debug("Shell detected at (" + std::to_string(x) + "," + std::to_string(y) + ")");
                    }
                }
                else{board[y][x] = '@';}
            }
            else if (!slot.getShells().empty()){
                if (!slot.getShells().empty()) {
                    if (verbose) {
                        Logger::debug("Shell detected at (" + std::to_string(x) + "," + std::to_string(y) + ")");
                    }
                    board[y][x] = '*';
                }
            }
        }
    }
    return board;
}

void GameManager_318772340_206580102::populateTankOrderAndLog(size_t rows, size_t cols) {
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

void GameManager_318772340_206580102::logGameResult() {
    if (!verbose) return;
    
    Logger::debug("Game Over!");

    if (tankIndex1 == 0 && tankIndex2 == 0)
        Logger::debug("Tie - Both players lost all tanks");
    else if (tankIndex1 == 0)
        Logger::debug("Player 2 wins - Player 1 eliminated");
    else if (tankIndex2 == 0)
        Logger::debug("Player 1 wins - Player 2 eliminated");
    else
        Logger::debug("Tie - Reached max steps or no shells left for all live tanks");
        
    if (!simpleOutput.is_open()) return;
    
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



} // namespace GameManager_318772340_206580102

// Create a typedef for registration
typedef GameManager_318772340_206580102::GameManager_318772340_206580102 GameManagerClass;

// Register the GameManager
REGISTER_GAME_MANAGER(GameManagerClass)
