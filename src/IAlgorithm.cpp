#include "IAlgorithm.h"
#include <iostream>

// A function to check if the tank in it's current position and direction can shoot in direct line without a wall and to hit the other tank
bool IAlgorithm::canShoot(const Tank& self, const Tank& enemy, const Board& board) {
    // if the tank can't shoot due to a cooldown or a laco of ammo, return false
    if (self.getAmmo() == 0 || self.getShootingStatus() > 0)
        return false;

    Position check = self.getPosition();
    Direction dir = self.getDirection();

    int counter = 0;
    while (true) {
        // tries to imitate a moving of the tank in the path of his direction
        check.move(dir, board.getWidth(), board.getHeight());

        // if the tank checked all of the cells in it's direction on the board, return false
        if (counter == std::max(board.getWidth(),board.getHeight())) {
            return false;
        }
        // if we got to the enemy's position, it means we have a direct path to him and we can shoot in order to hit him
        if (check == enemy.getPosition()) {
            return true;
        }

        // if the path has a wall, then we don't have a clear path to shoot
        const CellSlot& slot = board.getSlot(check.x, check.y);
        if (slot.getWall()) {
            return false;
        }
        counter++;
    }

}

// returns the rotation function needed to be in the desired direction from the curren direction
// will be useful to check if we can rotate towards an enemy 
ActionRequest IAlgorithm::rotateTowards(Direction::Value current, Direction::Value desired) const {
    int diff = (static_cast<int>(desired) - static_cast<int>(current) + 8) % 8;
    if (diff == 0) return ActionRequest::DoNothing;
    if (diff == 1) return ActionRequest::RotateRight45;
    if (diff == 2) return ActionRequest::RotateRight90;
    if (diff == 3) return ActionRequest::RotateRight90;
    if (diff == 4) return ActionRequest::RotateRight90;
    if (diff == 5) return ActionRequest::RotateLeft90;
    if (diff == 6) return ActionRequest::RotateLeft90;
    if (diff == 7) return ActionRequest::RotateLeft45;

    return ActionRequest::DoNothing;
}

// in case the tank is threatened by shells, it will try to move to a safe place or rotate towards a safe place
Action IAlgorithm::moveIfThreatened(const Board& board, const Tank& self) {
    Position myPos = self.getPosition();
    Direction::Value myDir = self.getDirection().getDirection();
    int width = board.getWidth();
    int height = board.getHeight();

    // we'll try first moving forward in the current direction
    Position forwardPos = myPos + Direction(myDir).toVector();
    forwardPos.x = (forwardPos.x + width) % width;
    forwardPos.y = (forwardPos.y + height) % height;

    const CellSlot& forwardSlot = board.getSlot(forwardPos.x, forwardPos.y);
    if (!forwardSlot.getWall() && !forwardSlot.getMine() && !forwardSlot.getTank()) {
        if (!isThreatenedByShells(board, forwardPos)) {
            return Action(ActionRequest::MoveForward);
        }
    }

    // if we can't move forward in the current direction, we'll find a safe cell around us and rotate towards it
    for (int i = 0; i < 8; ++i) {
        Direction::Value tryDir = static_cast<Direction::Value>(i);
        Position tryPos = myPos + Direction(tryDir).toVector();
        tryPos.x = (tryPos.x + width) % width;
        tryPos.y = (tryPos.y + height) % height;

        const CellSlot& trySlot = board.getSlot(tryPos.x, tryPos.y);
        if (!trySlot.getWall() && !trySlot.getMine() && !trySlot.getTank()) {
            if (!isThreatenedByShells(board, tryPos)) {
                if (myDir != tryDir) {
                    return Action(rotateTowards(myDir, tryDir));
                }
            }
        }
    }

    // if we have nothing safe to do, just stay in place
    return Action(ActionRequest::DoNothing);
}

// checks if the tans has one shell or more coming in his direction, and they are a threat because they will hit him if he stays in place
bool IAlgorithm::isThreatenedByShells(const Board& board, const Position& pos) {
    int width = board.getWidth();
    int height = board.getHeight();
    // for each shell in the board, check if it has a clear path to the tank and could hit him in the future
    for (Shell* shell : board.getShells()) {
        Position sPos = shell->getPosition();
        Direction::Value sDir = shell->getDirection().getDirection();
        Position moveVec = Direction(sDir).toVector();
        Position current = sPos;

        for (int i = 0; i < std::max(width, height); ++i) {
            if (current == pos) {
                return true;
            }

            // checks first if there's a wall in the path of the shell
            CellSlot& slot = board.getSlot(current.x, current.y);
            if (slot.getWall()) {
                break;  // the shell can't continue due to the wall, it's not a threat
            }

            current.x = (current.x + moveVec.x + width) % width;
            current.y = (current.y + moveVec.y + height) % height;
        }
    }

    return false;
}

// Returns the direction we need in order to get from one point to another points
Direction::Value IAlgorithm::getDirectionTo(const Position& from, const Position& to) const {
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

