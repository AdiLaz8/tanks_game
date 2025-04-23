#include "Algorithm2.h"
#include <iostream>
#include "Logger.h"


Direction::Value Algorithm2::getDirectionTo(const Position& from, const Position& to) const {
    int dx = to.x - from.x;
    int dy = to.y - from.y;
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

ActionType Algorithm2::rotateTowards(Direction::Value current, Direction::Value desired) const {
    int diff = (static_cast<int>(desired) - static_cast<int>(current) + 8) % 8;
    if (diff == 0) return ActionType::None;
    if (diff == 1) return ActionType::RotateRight8;
    if (diff == 2 || diff == 3 || diff == 4) return ActionType::RotateRight4;
    if (diff == 5 || diff == 6) return ActionType::RotateLeft4;
    if (diff == 7) return ActionType::RotateLeft8;
    return ActionType::None;
}

Action Algorithm2::nextAction(const Board& board, const Tank& self, const Tank& enemy) {
    if (isThreatenedByShells(board, self.getPosition())) {
        return moveIfThreatened(board, self);
    }
    
    if (canShoot(self, enemy, board) && self.getShootingStatus() == 0 && self.getAmmo() > 0) {
        Logger::debug("Algorithm2: Enemy in direct line of fire. Shooting now.");
        return Action(ActionType::Shoot);
    }
    if (enemy.getAmmo() == 0) {
        Direction::Value to = getDirectionTo(self.getPosition(), enemy.getPosition());
        Direction::Value from = self.getDirection().getDirection();
        if (from == to && self.getAmmo() > 0) {
            return Action(ActionType::Shoot);
        }
        return Action(rotateTowards(from, to));
    }
    // Direction::Value to = getDirectionTo(self.getPosition(), enemy.getPosition());
    // Direction::Value from = self.getDirection().getDirection();
    // if (from != to) {
    //     Logger::debug("Algorithm2: Rotating to face enemy.");
    //     return Action(rotateTowards(from, to));
    // }
    return Action(ActionType::None);
}
