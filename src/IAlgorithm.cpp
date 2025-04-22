#include "IAlgorithm.h"
#include <iostream>

bool IAlgorithm::canShoot(const Tank& self, const Tank& enemy, const Board& board) {
    if (self.getAmmo() == 0 || self.getShootingStatus() > 0)
        return false;

    Position check = self.getPosition();
    Direction dir = self.getDirection();

    int counter = 0;
    while (true) {
        check.move(dir, board.getWidth(), board.getHeight());

        if (counter == std::max(board.getWidth(),board.getHeight())) {
            return false;
        }

        if (check == enemy.getPosition()) {
            return true;
        }

        const CellSlot& slot = board.getSlot(check.x, check.y);
        if (slot.getWall()) {
            return false;
        }
        counter++;
    }

}


ActionType IAlgorithm::rotateTowards(Direction::Value current, Direction::Value desired) const {
    int diff = (static_cast<int>(desired) - static_cast<int>(current) + 8) % 8;
    if (diff == 0) return ActionType::None;
    if (diff == 1) return ActionType::RotateRight8;
    if (diff == 2) return ActionType::RotateRight4;
    if (diff == 3) return ActionType::RotateRight4;
    if (diff == 4) return ActionType::RotateRight4;
    if (diff == 5) return ActionType::RotateLeft4;
    if (diff == 6) return ActionType::RotateLeft4;
    if (diff == 7) return ActionType::RotateLeft8;

    return ActionType::None;
}
Action IAlgorithm::moveIfThreatened(const Board& board, const Tank& self) {
    Position myPos = self.getPosition();
    int width = board.getWidth();
    int height = board.getHeight();

    for (int i = 0; i < 8; ++i) {
        Direction::Value tryDir = static_cast<Direction::Value>(i);
        Position tryPos = myPos + Direction(tryDir).toVector();
        if (tryPos.x < 0 || tryPos.x >= width || tryPos.y < 0 || tryPos.y >= height)
            continue;

        const CellSlot& trySlot = board.getSlot(tryPos.x, tryPos.y);
        if (!trySlot.getWall() && !trySlot.getMine() && !trySlot.getTank()) {
            if (self.getDirection().getDirection() != tryDir)
                return Action(rotateTowards(self.getDirection().getDirection(), tryDir));
        }
    }

    return Action(ActionType::None);
}

bool IAlgorithm::isThreatenedByShells(const Board& board, const Position& pos) {
    int width = board.getWidth();
    int height = board.getHeight();

    for (Shell* shell : board.getShells()) {
        Position sPos = shell->getPosition();
        Direction::Value sDir = shell->getDirection().getDirection();
        Position moveVec = Direction(sDir).toVector();

        // נבנה מסלול של הפגז עד שהוא יפגע במשהו
        Position current = sPos;

        for (int i = 0; i < std::max(width, height); ++i) {
            // בדוק האם הפוזיציה הנוכחית של הפגז שווה לזו של הטנק
            if (current == pos) {
                return true;
            }

            current.x = (current.x + moveVec.x + width) % width;
            current.y = (current.y + moveVec.y + height) % height;

            // תוכל להוסיף כאן בדיקה אם הפגז נעצר בגלל קיר/מוקש
        }
    }

    return false;
}

