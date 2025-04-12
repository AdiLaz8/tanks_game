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
            std::cout << "Direct hit on enemy at position: " << check.x << ", " << check.y << "\n";
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
