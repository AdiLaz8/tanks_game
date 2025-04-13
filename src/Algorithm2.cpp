#include "Algorithm2.h"
#include <iostream>
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
    if(enemy.getAmmo()==0){
        Direction to=getDirectionTo(self.getPosition(),enemy.getPosition());
        Direction from=self.getDirection();
        if(self.getDirection()==to && self.getAmmo()>=0){
            return Action(ActionType::Shoot);
        }
        ActionType todo=rotateTowards(from.getDirection(),to.getDirection());
        return Action(todo);
    
    }
    Position myPos = self.getPosition();
    int width = board.getWidth();
    int height = board.getHeight();

    for (Shell* shell : board.getShells()) {
        Position sPos = shell->getPosition();
        Direction::Value sDir = shell->getDirection().getDirection();

        int dx = myPos.x - sPos.x;
        int dy = myPos.y - sPos.y;

        // האם הפגז בכלל באותו קו או אלכסון
        if ((dx == 0 || dy == 0 || abs(dx) == abs(dy))) {
            Position shellNext = sPos + Direction(sDir).toVector();
            int dShellX = shellNext.x - sPos.x;
            int dShellY = shellNext.y - sPos.y;

            // האם הפגז נע בכיוון של הטנק
            if ((dx * dShellX >= 0) && (dy * dShellY >= 0)) {
                // נבדוק אם תזוזה קדימה מקרבת אותנו לפגז – ואז נימנע ממנה
                Position forward = myPos + self.getDirection().toVector();
                if (forward.x >= 0 && forward.x < width && forward.y >= 0 && forward.y < height) {
                    // int currDist = abs(dx) + abs(dy);
                    // int nextDist = abs(forward.x - sPos.x) + abs(forward.y - sPos.y);

                    const CellSlot& fSlot = board.getSlot(forward.x, forward.y);
                    bool forwardSafe = !fSlot.getWall() && !fSlot.getMine() && !fSlot.getTank();

                    if (forwardSafe && !(self.getDirection().getOppositeDirection() == shell->getDirection())) {
                        return Action(ActionType::MoveForward);
                    }
                }

                // נבחר כיוון בטוח אחר
                for (int i = 0; i < 8; ++i) {
                    Direction::Value tryDir = static_cast<Direction::Value>(i);
                    Position tryPos = myPos + Direction(tryDir).toVector();
                    if (tryPos.x < 0 || tryPos.x >= width || tryPos.y < 0 || tryPos.y >= height) {
                        continue;
                    }
                    const CellSlot& trySlot = board.getSlot(tryPos.x, tryPos.y);
                    if (!trySlot.getWall() && !trySlot.getMine() && !trySlot.getTank()) {
                        if (self.getDirection().getDirection() != tryDir) { // אם הכיוון הנוכחי שונה
                            return Action(rotateTowards(self.getDirection().getDirection(), tryDir));
                        }
                    }
                }

                return Action(ActionType::None); // אין לאן לברוח
            }
        }
    }


    // אין איום – לא עושה כלום או תוסיף לוגיקה רגילה
    return Action(ActionType::None);
}

ActionType rotateTowards(Direction::Value current, Direction::Value desired) {
    if (current == desired) return ActionType::None;
    int diff = (desired - current + 8) % 8;
    if (diff > 4) {
        return (diff == 5) ? ActionType::RotateLeft4 : ActionType::RotateLeft8;
    } else {
        return (diff == 1) ? ActionType::RotateRight8 : ActionType::RotateRight4;
    }
}