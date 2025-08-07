#include "TankAlgorithm_318772340_206580102.h"
#include "../UserCommon/Logger.h"
#include <cmath>
#include <algorithm>
#include "MyBattleInfo.h"
#include <vector>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <queue>
#include <set>
#include "TankAlgorithmRegistration.h"

using namespace UserCommon_318772340_206580102;

namespace Algorithm_318772340_206580102 {

TankAlgorithm_318772340_206580102::TankAlgorithm_318772340_206580102(int playerIndex, int tankIndex)
    : playerId(playerIndex), tankId(tankIndex),
      direction((playerIndex == 1) ? Direction::L : Direction::R),selfPosition(-1,-1) {}

// checking if there's a mine in the position
bool TankAlgorithm_318772340_206580102::isMine(const Position& pos) const {
    for (const auto& mine : minePositions) {
        if (mine == pos) {
            return true;
        }
    }
    return false;
}

int TankAlgorithm_318772340_206580102::getTankId() const { return tankId; }
Direction TankAlgorithm_318772340_206580102::getTankDirection() const { return direction; }
Position TankAlgorithm_318772340_206580102::getTankPosition() const { return selfPosition; }
int TankAlgorithm_318772340_206580102::getAmmo() const { return ammo; }
int TankAlgorithm_318772340_206580102::getShootingStatus() const { return shootingStatus; }
int TankAlgorithm_318772340_206580102::getBackwardStatus() const { return backwardStatus; }
void TankAlgorithm_318772340_206580102::decreaseShooting() {
    if (shootingStatus > 0) shootingStatus--;
}

void TankAlgorithm_318772340_206580102::decreaseBackward() {
    if (backwardStatus > 0) backwardStatus--;
}

void TankAlgorithm_318772340_206580102::updateBattleInfo(BattleInfo& info) {
    auto& myInfo = dynamic_cast<MyBattleInfo&>(info);
    
    // First turn initialization for both players
    if (turnCounterSinceInfo == -1) {
        boardWidth = myInfo.getWidth();
        boardHeight = myInfo.getHeight();
        ammo = myInfo.getInitialShells();
    }
    
    turnCounterSinceInfo = 0;
    fullView = myInfo.getFullView();
    selfPosition = myInfo.getSelfPosition();
    
    if (playerId == 1) {
        // Player 1 specific logic (from Algo1)
        needsNewInfo = false;
        char enemySymbol = '2';
        
        // Calculate closest enemy tank
        int minDist = boardWidth * boardHeight;
        for (const auto& [pos, symbol] : fullView) {
            if (symbol == enemySymbol) {
                int dx = std::min((selfPosition.getx() - pos.getx() + boardWidth) % boardWidth,
                                  (pos.getx() - selfPosition.getx() + boardWidth) % boardWidth);
                int dy = std::min((selfPosition.gety() - pos.gety() + boardHeight) % boardHeight,
                                  (pos.gety() - selfPosition.gety() + boardHeight) % boardHeight);
                int dist = dx + dy;
                if (dist < minDist) {
                    minDist = dist;
                    targetPos = pos;
                }
            }
        }
        
        // Initialize mines list
        if (!minesInitialized) {
            minePositions.clear();
            for (const auto& [pos, symbol] : fullView) {
                if (symbol == '@') {
                    minePositions.push_back(pos);
                }
            }
            minesInitialized = true;
        }
        computeShootingPath();
    }
    
    // Player 2 doesn't need additional logic beyond the common initialization
}

//searching for shells in the 2X2 slots around our tank, if so we are threatened
bool TankAlgorithm_318772340_206580102::isThreatenedByShells() const {
    for (const auto& [pos, symbol] : fullView) {
        if (symbol == '*') {
            int dx = std::min((int)(pos.getx() - selfPosition.getx() + boardWidth) % (int)boardWidth,
                              (int)(selfPosition.getx() - pos.getx() + boardWidth) % (int)boardWidth);
            int dy = std::min((int)(pos.gety() - selfPosition.gety() + boardHeight) % (int)boardHeight,
                              (int)(selfPosition.gety() - pos.gety() + boardHeight) % (int)boardHeight);
            if (dx <= 2 && dy <= 2){
                 return true;
            }
        }
    }
    return false;
}

//if possible to move forward-->move , if not then rotating towards a direction its possible to move forword there
Action TankAlgorithm_318772340_206580102::moveIfThreatened() {
    Position forward = selfPosition + direction.toVector();
    forward.setx((forward.getx() + boardWidth) % boardWidth);
    forward.sety((forward.gety() + boardHeight) % boardHeight);
    auto isFree = [&](const Position& pos) {
        for (const auto& [p, sym] : fullView) {
            if ((p == pos && sym != ' ')) return false;
        }
        return true;
    };
    if (isFree(forward) && !isMine(forward)) {
        selfPosition = forward;
        moveAfterRotate = false;
        return Action(ActionRequest::MoveForward);
    }
    int currIndex = static_cast<int>(direction.getDirection());
    std::vector<int> offsets = {1, 2, -1, -2};

    for (int offset : offsets) {
        int tryIndex = (currIndex + offset + 8) % 8;
        Direction::Value tryDir = static_cast<Direction::Value>(tryIndex);
        Position delta = Direction(tryDir).toVector();
        Position candidate = selfPosition + delta;
        candidate.setx((candidate.getx() + boardWidth) % boardWidth);
        candidate.sety((candidate.gety() + boardHeight) % boardHeight);
        if (isFree(candidate) && !isMine(candidate)) {
            moveAfterRotate = true;
            return Action(rotateTowards(direction.getDirection(), tryDir));
        }
    }
    return Action(ActionRequest::GetBattleInfo);
}

//if an enemy is in sight, and no friendly tank in the way
bool TankAlgorithm_318772340_206580102::canShootInDirection() const {
    char enemySymbol = (playerId == 1 ? '2' : '1');
    char selfSymbol = (playerId == 1 ? '1' : '2');
    Position ray = selfPosition;
    for (size_t i = 0; i < std::max(boardWidth, boardHeight); ++i) {
        ray = ray + direction.toVector();
        ray.setx((ray.getx() + boardWidth) % boardWidth);
        ray.sety((ray.gety() + boardHeight) % boardHeight);
        if (ray == selfPosition) break;
        for (const auto& [pos, symbol] : fullView) {
            if (pos == ray) {
                if (symbol == selfSymbol) {
                    return false;
                }
                if (symbol == enemySymbol) {
                    return true;
                }
            }
        }
    }
    return false;
}

// same function but for different directions (we use that in the for loop with all directions)
bool TankAlgorithm_318772340_206580102::canShootInDirection(Direction dir) const {
    Position pos = selfPosition;
    for (int i = 0; i < 4; ++i) {
        pos.move(dir, boardWidth, boardHeight);
        char symbol = ' ';
        for (const auto& [p, c] : fullView) {
            if (p == pos) {
                symbol = c;
                break;
            }
        }
        if (symbol == ' ') continue;                
        if (symbol == '#') return false;       
        if (symbol == selfSymbol()) return false;  
        if (symbol == enemySymbol()) return true; 
    }
    return false; 
}

// get the direction we need to get from pusition 'from' to position 'to'
Direction::Value TankAlgorithm_318772340_206580102::getDirectionTo(const Position& from, const Position& to) const {
    int dx = ((to.getx() - from.getx() + boardWidth) % boardWidth + boardWidth / 2) % boardWidth - boardWidth / 2;
    int dy = ((to.gety() - from.gety() + boardHeight) % boardHeight + boardHeight / 2) % boardHeight - boardHeight / 2;
    int dirX = (dx > 0) ? 1 : (dx < 0 ? -1 : 0);
    int dirY = (dy > 0) ? 1 : (dy < 0 ? -1 : 0);
    if (dirX == 0 && dirY == -1) return Direction::U;
    if (dirX == 1 && dirY == -1) return Direction::UR;
    if (dirX == 1 && dirY == 0) return Direction::R;
    if (dirX == 1 && dirY == 1) return Direction::DR;
    if (dirX == 0 && dirY == 1) return Direction::D;
    if (dirX == -1 && dirY == 1) return Direction::DL;
    if (dirX == -1 && dirY == 0) return Direction::L;
    if (dirX == -1 && dirY == -1) return Direction::UL;
    return Direction::U;
}

// rotate the tank towards a desired direction, return the action request
ActionRequest TankAlgorithm_318772340_206580102::rotateTowards(Direction::Value current, Direction::Value desired){
    int diff = (static_cast<int>(desired) - static_cast<int>(current) + 8) % 8;
    if (diff == 0) return ActionRequest::GetBattleInfo;
    if (diff == 1){
        direction.rotateClockwise8();
        return ActionRequest::RotateRight45;
    }
    if (diff == 2 || diff == 3 || diff == 4){
        direction.rotateClockwise4();
        return ActionRequest::RotateRight90;
    }
    if (diff == 5 || diff == 6){
        direction.rotateCounterClockwise4();
        return ActionRequest::RotateLeft90;
    }
    if (diff == 7){
        direction.rotateCounterClockwise8();
        return ActionRequest::RotateLeft45;
    }
    return ActionRequest::GetBattleInfo;
}

void TankAlgorithm_318772340_206580102::setBackward(int i){
    backwardStatus = i;
}

// Main action dispatcher based on player ID
ActionRequest TankAlgorithm_318772340_206580102::getAction() {
    std::cout << "DEBUG: TankAlgorithm::getAction() called for Player " << playerId << " Tank " << tankId << std::endl;
    ActionRequest result;
    if (playerId == 1) {
        result = getPlayer1Action();
    } else {
        result = getPlayer2Action();
    }
    std::cout << "DEBUG: TankAlgorithm::getAction() returning " << static_cast<int>(result) << " for Player " << playerId << " Tank " << tankId << std::endl;
    return result;
}

// Player 1 action logic (from Algo1)
ActionRequest TankAlgorithm_318772340_206580102::getPlayer1Action() {
    ActionRequest action = ActionRequest::DoNothing;
    if (turnCounterSinceInfo == -1 || turnCounterSinceInfo == 5) {
        Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": No battle info yet or 5 turns passed since last update, requesting BattleInfo to refresh awareness.");
        return ActionRequest::GetBattleInfo;
    }
    if(getShootingStatus()>0){
        shootingStatus--;
    }
    action = moveForwardAfterRotate();
    if (action != ActionRequest::DoNothing) {
        return action;
    }
    if (needsNewBattleInfo()) {
        chasing = false;
        turnCounterSinceInfo = 0;
        Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": Current path invalidated or target lost, requesting BattleInfo for updated state.");
        return ActionRequest::GetBattleInfo;
    }
    action = getShootingActionIfAvailable();
    if (action != ActionRequest::DoNothing) {
        return action;
    }
    // try to get an action from the BFS path
    action = getActionFromPath();
    if (action != ActionRequest::DoNothing) {
        return action;
    }
    turnCounterSinceInfo = 0;
    chasing = false;
    Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": Info outdated or chase failed, requesting updated battle info.");
    return ActionRequest::GetBattleInfo;
}

// Player 2 action logic (from Algo2)
ActionRequest TankAlgorithm_318772340_206580102::getPlayer2Action() {
    std::cout << "DEBUG: getPlayer2Action() called for Tank " << tankId << ", turnCounterSinceInfo=" << turnCounterSinceInfo << std::endl;
    if (turnCounterSinceInfo == -1) { // first turn
        std::cout << "DEBUG: Player 2 Tank " << tankId << ": First turn, requesting battle info." << std::endl;
        Logger::debug("Player 2: Tank " + std::to_string(tankId) + ": First turn, requesting battle info.");
        return ActionRequest::GetBattleInfo;
    }
    turnCounterSinceInfo++;

    if (getShootingStatus() > 0) {
        shootingStatus--;
    }

    // If the tank is threatened and rotated last turn so now it needs to move forward
    if (moveAfterRotate) {
        moveAfterRotate = false;
        Position newPos = selfPosition + direction.toVector();
        newPos.setx((newPos.getx() + boardWidth) % boardWidth);
        newPos.sety((newPos.gety() + boardHeight) % boardHeight);
        selfPosition = newPos;
        Logger::debug("Player 2: Tank " + std::to_string(tankId) + ": Moved forward after rotation to avoid shell threat that was detected the turn before.");
        return ActionRequest::MoveForward;
    }

    if (isThreatenedByShells()) { // if the tank is threatened by shells, try to escape
        Logger::debug("Player 2: Tank " + std::to_string(tankId) + ": Shell threat detected, initiating evasive move.");
        return moveIfThreatened().getType();
    }

    if (canShootInDirection() && ammo > 0 && shootingStatus == 0) { // if enemy in sight --> shoot
        shootingStatus = 5;
        ammo--;
        Logger::debug("Player 2: Tank " + std::to_string(tankId) + ": Enemy in line of sight, firing.");
        return ActionRequest::Shoot;
    }

    Logger::debug("Player 2: Tank " + std::to_string(tankId) + ": Moving forward by default.");
    return ActionRequest::MoveForward;
}

// Player 1 specific methods (from Algo1)
bool TankAlgorithm_318772340_206580102::needsNewBattleInfo() const {
    return needsNewInfo || turnCounterSinceInfo >= 5 || (chasing && currentPath.empty());
}

ActionRequest TankAlgorithm_318772340_206580102::getShootingActionIfAvailable() {
    bool canShootNow = canShootInDirection();
    //if enemy in sight --> shoot
    if (canShootNow) {
        if (ammo > 0 && getShootingStatus()==0) {
            shootingStatus=5;
            ammo--;
            turnCounterSinceInfo++;
            needsNewInfo = true;
            Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": Enemy in direct line of sight, firing in direction "+ direction.toString());
            return ActionRequest::Shoot;
        }
    }
    //searching for enemy in sight in one of the directions
    for (int i = 0; i < 8; ++i) {
        if (i == static_cast<int>(direction.getDirection())) continue;
        Direction tryDir(static_cast<Direction::Value>(i));
        Direction originalDir = direction;
        direction = tryDir;
        if (canShootInDirection()) {
            direction = originalDir; 
            turnCounterSinceInfo++;
            Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": Enemy in other line of sight at , rotating to direction " + tryDir.toString());
            return rotateTowards(originalDir.getDirection(), tryDir.getDirection());
        }
        direction = originalDir; 
    }
    return ActionRequest::DoNothing;
}

ActionRequest TankAlgorithm_318772340_206580102::moveForwardAfterRotate() {
    // if it's still threatened by shells, we need to move
    if (isThreatenedByShells()) {
        currentPath.clear();
        turnCounterSinceInfo++;
        Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": Shell threat detected, taking evasive action.");
        return moveIfThreatened().getType();
    }
    // if we rotated last turn, we move forward if there's no mine in front
    if (moveAfterRotate) {
            moveAfterRotate = false;
            needsNewInfo = true;
            turnCounterSinceInfo++;
            Position newPos = selfPosition + direction.toVector();
            newPos.setx((newPos.getx() + boardWidth) % boardWidth);
            newPos.sety((newPos.gety() + boardHeight) % boardHeight);
            if (!isMine(newPos)) {
                selfPosition = newPos;
                Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": Shell threat detected, taking evasive action.");
                return ActionRequest::MoveForward;
            }
        }
    return ActionRequest::DoNothing;
}

ActionRequest TankAlgorithm_318772340_206580102::getActionFromPath() {
    std::cout << "DEBUG: getActionFromPath() called for Player " << playerId << " Tank " << tankId << std::endl;

    if (!currentPath.empty()) {
        std::cout << "DEBUG: Path not empty, processing next direction" << std::endl;
        Direction::Value nextDir = currentPath.front();
        std::cout << "DEBUG: Next direction: " << static_cast<int>(nextDir) << std::endl;
        
        if (nextDir == direction.getDirection()) {
            std::cout << "DEBUG: Direction matches, checking next position" << std::endl;
            Position nextPos = selfPosition + Direction(nextDir).toVector();
            nextPos.setx((nextPos.getx() + boardWidth) % boardWidth);
            nextPos.sety((nextPos.gety() + boardHeight) % boardHeight);
            std::cout << "DEBUG: Next position: (" << nextPos.getx() << "," << nextPos.gety() << ")" << std::endl;
            
            auto it = std::find_if(fullView.begin(), fullView.end(),[&](const auto& cell) {return cell.first == nextPos;});
            if (it != fullView.end()) {
                std::cout << "DEBUG: Found cell at next position: '" << it->second << "'" << std::endl;
                if (it->second == '@' || it->second == '1' || it->second == '2') {
                    std::cout << "DEBUG: Path blocked by mine/tank, cancelling path" << std::endl;
                    currentPath.clear();
                    needsNewInfo = true;
                    turnCounterSinceInfo = 0;
                    Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": Path blocked (mine/tank/wall), cancelling path and requesting battle info.");
                    return ActionRequest::GetBattleInfo;
                }
                if (it->second == '#') {
                    if (ammo > 0 && getShootingStatus() == 0) {
                        std::cout << "DEBUG: Wall in the way, firing to destroy it" << std::endl;
                        shootingStatus = 4;
                        ammo--;
                        needsNewInfo = true;
                        turnCounterSinceInfo++;
                        Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": wall in the way, firing to destroy it. firing in direction "+ direction.toString());
                        return ActionRequest::Shoot;
                    }
                }
            }
            currentPath.erase(currentPath.begin());
            if (!isMine(nextPos)) {
                std::cout << "DEBUG: Moving forward to next position" << std::endl;
                selfPosition = nextPos;
                turnCounterSinceInfo++;
                Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": Advancing along BFS path towards target.");
                return ActionRequest::MoveForward;
            } else {
                std::cout << "DEBUG: Mine detected, cancelling path" << std::endl;
                currentPath.clear();
                chasing = false;
                turnCounterSinceInfo = 0;
                Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": Requesting GetBattleInfo to refresh map state.");
                return ActionRequest::GetBattleInfo;
            }
        } else {
            std::cout << "DEBUG: Direction doesn't match, rotating towards next direction" << std::endl;
            direction = Direction(nextDir);
            moveAfterRotate = true;
            turnCounterSinceInfo++;
            Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": Rotating towards next direction in path: " + Direction(nextDir).toString());
            return rotateTowards(direction.getDirection(), nextDir);
        }
    }
    std::cout << "DEBUG: No path available, returning DoNothing" << std::endl;
    return ActionRequest::DoNothing; // No actions available from the path
}

void TankAlgorithm_318772340_206580102::computeShootingPath() {
    std::cout << "DEBUG: computeShootingPath() called for Player " << playerId << " Tank " << tankId << std::endl;
    std::cout << "DEBUG: selfPosition=(" << selfPosition.getx() << "," << selfPosition.gety() << "), targetPos=(" << targetPos.getx() << "," << targetPos.gety() << ")" << std::endl;
    std::cout << "DEBUG: boardWidth=" << boardWidth << ", boardHeight=" << boardHeight << std::endl;
    currentPath = computeBFS(selfPosition, targetPos, fullView, boardWidth, boardHeight);
    std::cout << "DEBUG: BFS completed, path size=" << currentPath.size() << std::endl;
    if (!currentPath.empty()) {
        Logger::debug("Player 1: Tank " + std::to_string(tankId) + ": Computed new path to target with " + std::to_string(currentPath.size()) + " steps.");
        chasing = true;
    }
}

std::vector<Direction::Value> TankAlgorithm_318772340_206580102::computeBFS(const Position& from, const Position& to,
                                                         const std::vector<std::pair<Position, char>>& fullView,
                                                         size_t width, size_t height) {
    std::cout << "DEBUG: computeBFS() called: from=(" << from.getx() << "," << from.gety() << "), to=(" << to.getx() << "," << to.gety() << ")" << std::endl;
    std::cout << "DEBUG: width=" << width << ", height=" << height << std::endl;
    
    // Print current board state for debugging
    std::cout << "DEBUG: Current board state:" << std::endl;
    for (size_t y = 0; y < height; ++y) {
        std::cout << "DEBUG: Row " << y << ": ";
        for (size_t x = 0; x < width; ++x) {
            Position pos(x, y);
            bool found = false;
            for (const auto& [boardPos, symbol] : fullView) {
                if (boardPos == pos) {
                    std::cout << symbol;
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::cout << ".";
            }
        }
        std::cout << std::endl;
    }
    
    (void)to; 
    std::set<Position> blocked;
    char friendSymbol = (playerId == 1 ? '1' : '2');
    char enemySymbol = (playerId == 1 ? '2' : '1');
    for (const auto& [pos, symbol] : fullView) {
        if (isMine(pos) || symbol == '@' || symbol == friendSymbol) // blocking mines and friendly tanks
            blocked.insert(pos);
    }
    std::queue<std::pair<Position, std::vector<Direction::Value>>> q;
    std::set<Position> visited;
    q.push({from, {}});
    visited.insert(from);
    while (!q.empty()) {
        auto [current, path] = q.front();
        q.pop();
        for (int i = 0; i < 8; ++i) {
            Direction::Value dirVal = static_cast<Direction::Value>(i);
            Position delta = Direction(dirVal).toVector();
            Position next = current + delta;
            next.setx((next.getx() + width) % width);
            next.sety((next.gety() + height) % height);
            if (visited.count(next) || blocked.count(next))
                continue;
            std::vector<Direction::Value> newPath = path;
            newPath.push_back(dirVal);
            Position step = next;
            Direction shootingDir(dirVal);
            for (size_t s = 0; s < std::max(width, height); ++s) {
                step.setx((step.getx() + width) % width);
                step.sety((step.gety() + height) % height);
                step = step + shootingDir.toVector();
                if (step == from)
                    break;
                for (const auto& [pos, symbol] : fullView) {
                    if (pos == step && symbol == enemySymbol) {
                        std::cout << "DEBUG: Found shooting position at (" << next.getx() << "," << next.gety() << ") with path size " << newPath.size() << std::endl;
                        return newPath;
                    }
                }
            }
            q.push({next, newPath});
            visited.insert(next);
        }
    }
    std::cout << "DEBUG: No path found" << std::endl;
    return {};
}

} // namespace Algorithm_318772340_206580102

// Register the TankAlgorithm (outside namespace)
typedef Algorithm_318772340_206580102::TankAlgorithm_318772340_206580102 TankAlgorithmClass;
REGISTER_TANK_ALGORITHM(TankAlgorithmClass)
