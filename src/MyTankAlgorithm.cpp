#include "MyTankAlgorithm.h"
#include <cmath>
#include <algorithm>
#include "MyBattleInfo.h"
#include <vector>
#include <utility>
#include <stdexcept>
#include <iostream>

MyTankAlgorithm::MyTankAlgorithm(int playerIndex, int tankIndex)
    : playerId(playerIndex), tankId(tankIndex),
      direction((playerIndex == 1) ? Direction::L : Direction::R),selfPosition(-1,-1) {}

bool MyTankAlgorithm::isMine(const Position& pos) const {
    for (const auto& mine : minePositions) {
        if (mine == pos) {
            std::cout << "[FOUND] Position " << pos.getx() << "," << pos.gety()
                      << " is a mine!" << std::endl;
            return true;
        }
    }
    return false;
}



int MyTankAlgorithm::getTankId() const { return tankId; }
Direction MyTankAlgorithm::getTankDirection() const { return direction; }
Position MyTankAlgorithm::getTankPosition() const { return selfPosition; }
int MyTankAlgorithm::getAmmo() const { return ammo; }
int MyTankAlgorithm::getShootingStatus() const { return shootingStatus; }
int MyTankAlgorithm::getBackwardStatus() const { return backwardStatus; }
void MyTankAlgorithm::decreaseShooting() {
    if (shootingStatus > 0) shootingStatus--;
}
void MyTankAlgorithm::decreaseBackward() {
    if (backwardStatus > 0) backwardStatus--;
}
void MyTankAlgorithm::updateBattleInfo(BattleInfo&) {
    throw std::runtime_error("updateBattleInfo must be overridden.");
}
//searching for shells in the 2X2 slots around our tank, if so we are threatened
bool MyTankAlgorithm::isThreatenedByShells() const {
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
Action MyTankAlgorithm::moveIfThreatened() {
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
        std::cout << "[MOVE1][TANK " << tankId << "] Moving forward to " 
          << forward.getx() << "," << forward.gety() << std::endl;
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
bool MyTankAlgorithm::canShootInDirection() const {
    char enemySymbol = (playerId == 1 ? '2' : '1');
    char selfSymbol = (playerId == 1 ? '1' : '2');
    Position ray = selfPosition;
    // std::cout << "[DEBUG] Player ID: " << playerId
    //       << ", selfSymbol: '" << selfSymbol
    //       << "', enemySymbol: '" << enemySymbol << "'" << std::endl;
    for (size_t i = 0; i < std::max(boardWidth, boardHeight); ++i) {
        // std::cout << "[DEBUG] direction = " << direction.getDirection() << " from position " << ray.getx()<<ray.gety() << std::endl;
        ray = ray + direction.toVector();
        // std::cout << "[DEBUG] direction = " << direction.getDirection() << " from position " << ray.getx()<<ray.gety() << std::endl;
        ray.setx((ray.getx() + boardWidth) % boardWidth);
        ray.sety((ray.gety() + boardHeight) % boardHeight);
        if (ray == selfPosition) break;
        for (const auto& [pos, symbol] : fullView) {
            if (pos == ray) {
                // std::cout << "[RAY] checking (" << ray.getx() << "," << ray.gety() << ") -> symbol: " << symbol << std::endl;
                if (symbol == selfSymbol) {
                    // std::cout << "[BLOCKED] Friend at (" << ray.getx() << "," << ray.gety() << ")" << std::endl;
                    return false;
                }
                if (symbol == enemySymbol) {
                    // std::cout << "[TARGET] Enemy at (" << ray.getx() << "," << ray.gety() << ")" << std::endl;
                    return true;
                }
            }
        }
    }
    return false;
}
// same function but for different directions (we use that in the for loop with all directions)
bool MyTankAlgorithm::canShootInDirection(Direction dir) const {
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
Direction::Value MyTankAlgorithm::getDirectionTo(const Position& from, const Position& to) const {
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
ActionRequest MyTankAlgorithm::rotateTowards(Direction::Value current, Direction::Value desired){
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
void MyTankAlgorithm::setBackward(int i){
    backwardStatus=i;
}
