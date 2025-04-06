#include "Algorithm2.h"

Action Algorithm2::nextAction(const Board& board, const Tank& self, const Tank& enemy) {
    // Check for incoming shells
    if (isThreatenedByShells(board, self.getPosition())) {
        Action safeMove = moveIfThreatened(board, self);
        if (safeMove.getType() != ActionType::None) {
            return safeMove;
        }
    }

    // Try to shoot if there is no immediate threat
    if (canShoot(self, enemy, board)) {
        return Action(ActionType::Shoot);
    }

    // If no threats and can't shoot, stay in position
    return Action(ActionType::None);
}

bool Algorithm2::isThreatenedByShells(const Board& board, const Position& pos) {
    for (const Shell* shell : board.getShells()) {
        Position shellPos = shell->getPosition();
        Direction shellDir = shell->getDirection();

        // Calculate the next two positions the shell will move to
        Position nextPos1 = shellPos + shellDir.toVector();  // First move
        Position nextPos2 = nextPos1 + shellDir.toVector();  // Second move

        // Check if either position collides with the tank's current position
        if (nextPos1 == pos || nextPos2 == pos) {
            return true;
        }
    }
    return false;
}

Action Algorithm2::moveIfThreatened(const Board& board, const Tank& self) {
    std::vector<Direction::Value> directions = {
        Direction::U, Direction::D, Direction::L, Direction::R,
        Direction::UL, Direction::UR, Direction::DL, Direction::DR
    };

    for (Direction::Value dir : directions) {
   
        Position nextPos = self.getPosition() + Direction(dir).toVector();

        // אם אפשר לעבור לשם והוא לא מוקש
        if (board.isPassable(nextPos.x, nextPos.y) &&
            !board.getSlot(nextPos.x, nextPos.y).getMine()) {

            // אם הטנק פונה לכיוון הזה — נבצע תזוזה
            if (dir == self.getDirection().getDirection()) {
                return Action(ActionType::MoveForward);
            }

            // אחרת נסתובב כדי לפנות לשם בתור הבא
            ActionType rotate = rotateTowards(self.getDirection().getDirection(), dir);
            return Action(rotate);
        }
    }

    return Action(ActionType::None); // לא נמצאה תזוזה בטוחה
}
