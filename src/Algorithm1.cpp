#include "Algorithm1.h"
#include <queue>
#include <set>
#include <unordered_map>
#include <cmath>
#include <utility>

Direction::Value Algorithm1::getDirectionTo(const Position& from, const Position& to) const {
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


std::vector<Direction::Value> Algorithm1::computeBFS(const Board& board, const Tank& self, const Tank& enemy) {
    struct Node {
        Position pos;
        std::vector<Direction::Value> path;
        int penalty;

        bool operator>(const Node& other) const {
            return penalty > other.penalty;
        }
    };

    int width = board.getWidth();
    int height = board.getHeight();

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    std::set<std::pair<int, int>> visited;

    pq.push({self.getPosition(), {}, 0});

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

        auto currentKey = std::make_pair(current.pos.x, current.pos.y);
        if (visited.count(currentKey)) continue;
        visited.insert(currentKey);

        // בודק אם אפשר לירות מכאן
        Tank tempTank(self.getSymbol(), self.getAmmo(), self.getDirection(), current.pos, self.getShootingStatus(), self.getBackwardStatus());
        if (canShoot(tempTank, enemy, board)) {
            return current.path;  // הצלחנו
        }

        // בודק את כל 8 הכיוונים
        for (int i = 0; i < 8; ++i) {
            Direction::Value dir = static_cast<Direction::Value>(i);
            Position nextPos = current.pos;
            nextPos.move(Direction(dir), width, height);

            auto nextKey = std::make_pair(nextPos.x, nextPos.y);
            if (visited.count(nextKey)) continue;

            CellSlot& slot = board.getSlot(nextPos.x, nextPos.y);

            // אם יש מוקש אי אפשר לעבור
            if (slot.getMine()) continue;

            // קיר זה מסלול חוקי אבל עם עונש
            int newPenalty = current.penalty;
            if (slot.getWall()) {
                newPenalty += 10;  // נעניש כדי ש-BFS יבחר רק אם אין ברירה
            }

            // מייצרים את המסלול המעודכן
            std::vector<Direction::Value> newPath = current.path;
            newPath.push_back(dir);

            pq.push({nextPos, newPath, newPenalty});
        }
    }

    return {};  // לא נמצא מסלול תקף
}
Action Algorithm1::nextAction(const Board& board,const Tank& self, const Tank& enemy){
    // 1. בדיקה אם אפשר לירות
    if (canShoot(self, enemy, board)) {
        return Action(ActionType::Shoot);
    }

    // 2. בדיקה אם המיקום של האויב השתנה או שאין מסלול
    if (currentPath.empty() || !(enemy.getPosition() == lastEnemyPos)) {
        currentPath = computeBFS(board, self, enemy);
        lastEnemyPos = enemy.getPosition();
    }

    // 3. אם אין מסלול — פשוט לא לזוז (או נוכל לכתוב התנהגות אחרת)
    if (currentPath.empty()) {
    // 🔥 ננסה לירות בקיר שנמצא מול הכיוון הנוכחי
        Position ahead = self.getPosition() + self.getDirection().toVector();
        const CellSlot& slot = board.getSlot(ahead.x, ahead.y);

        if (slot.getWall() && self.getShootingStatus() == 0 && self.getAmmo() > 0) {
            return Action(ActionType::Shoot);
        }

    // אם אין קיר לירות עליו, לא נעשה כלום
    return Action(ActionType::None);
}

    // 4. פעולה הבאה במסלול
    Direction::Value targetDir = currentPath.front();

    // 5. אם הטנק כבר פונה לכיוון הנכון — נתקדם קדימה ונמחק את הצעד
    if (self.getDirection().getDirection() == targetDir) {
        if (canShoot(self, enemy, board)) {
            return Action(ActionType::Shoot);
    }
        currentPath.erase(currentPath.begin());
        return Action(ActionType::MoveForward);
    }

    // 6. אחרת — נסובב לכיוון המתאים (נשאיר את הצעד לתור הבא)
    ActionType turn = rotateTowards(self.getDirection().getDirection(), targetDir);
    return Action(turn);

}
ActionType Algorithm1::rotateTowards(Direction::Value current, Direction::Value desired) const {
    int cur = static_cast<int>(current);
    int des = static_cast<int>(desired);
    int diff = (des - cur + 8) % 8;

    if (diff == 0) return ActionType::None;
    if (diff <= 4) return (diff == 1) ? ActionType::RotateRight8 :
                         (diff == 2) ? ActionType::RotateRight4 : ActionType::RotateRight4;
    else return (8 - diff == 1) ? ActionType::RotateLeft8 :
                  (8 - diff == 2) ? ActionType::RotateLeft4 : ActionType::RotateLeft4;
}