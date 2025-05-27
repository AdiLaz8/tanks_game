// MyTankAlgorithm.cpp
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
      direction((playerIndex == 1) ? Direction::R : Direction::L),selfPosition(-1,-1) {}



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
    // או ריק, או שגיאה אם באמת לא אמור להיקרא
    throw std::runtime_error("updateBattleInfo must be overridden.");
}

bool MyTankAlgorithm::isThreatenedByShells() const {
    for (const auto& [pos, symbol] : fullView) {

        if (symbol == '*') {
            //std::cout << "Seeing symbol '" << symbol << std::endl;
            int dx = std::min((int)(pos.getx() - selfPosition.getx() + boardWidth) % (int)boardWidth,
                              (int)(selfPosition.getx() - pos.getx() + boardWidth) % (int)boardWidth);
            int dy = std::min((int)(pos.gety() - selfPosition.gety() + boardHeight) % (int)boardHeight,
                              (int)(selfPosition.gety() - pos.gety() + boardHeight) % (int)boardHeight);
            if (dx <= 2 && dy <= 2){
                //std::cout << "Threat detected " << std::endl;
                 return true;
            }
        }
    }
    return false;
}

Action MyTankAlgorithm::moveIfThreatened() {
    Position forward = selfPosition + direction.toVector();
    forward.setx((forward.getx() + boardWidth) % boardWidth);
    forward.sety((forward.gety() + boardHeight) % boardHeight);

    auto isFree = [&](const Position& pos) {
        for (const auto& [p, sym] : fullView) {
            if (p == pos && sym != ' ') return false;
        }
        return true;
    };

    if (isFree(forward)){
        selfPosition = forward;
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

        if (isFree(candidate)) {
            moveAfterRotate = true;
            return Action(rotateTowards(direction.getDirection(), tryDir));
        }
    }

    return Action(ActionRequest::GetBattleInfo);
}

bool MyTankAlgorithm::canShootInDirection() const {
    char enemySymbol = (playerId == 1 ? '2' : '1');
    char selfSymbol = (playerId == 1 ? '1' : '2');
    Position ray = selfPosition;
    for (size_t i = 0; i < std::max(boardWidth, boardHeight); ++i) {
        ray = ray + direction.toVector();
        ray.setx((ray.getx() + boardWidth) % boardWidth);
        ray.sety((ray.gety() + boardHeight) % boardHeight);
        if (ray == selfPosition) break;
        for (size_t i = 0; i < std::max(boardWidth, boardHeight); ++i) {
            ray = ray + direction.toVector();
            ray.setx((ray.getx() + boardWidth) % boardWidth);
            ray.sety((ray.gety() + boardHeight) % boardHeight);

            if (ray == selfPosition) break;


            // בדוק אם יש משהו במשבצת הזו
            for (const auto& [pos, symbol] : fullView) {
                if (pos == ray) {
                    //std::cout << "Seeing symbol '" << symbol << std::endl;

                    if (symbol == selfSymbol){
                        std::cout << "i am '" << selfSymbol << std::endl;
                        std::cout << "no shoot '" << symbol << std::endl;
                        std::cout << "my friend in position '" << ray.getx() <<ray.gety()<< std::endl;
                        std::cout << "im in position '" << selfPosition.getx() <<selfPosition.gety()<< std::endl;

                        return false;   // טנק שלי בדרך
                    }
                    if (symbol == enemySymbol) return true;   // טנק אויב בדרך
                }
            }
        }
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
