#include "Algorithm1.h"
#include <queue>
#include <set>
#include <iostream>

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

ActionType Algorithm1::rotateTowards(Direction::Value current, Direction::Value desired) const {
    int diff = (static_cast<int>(desired) - static_cast<int>(current) + 8) % 8;
    if (diff == 0) return ActionType::None;
    if (diff == 1) return ActionType::RotateRight8;
    if (diff == 2 || diff == 3 || diff == 4) return ActionType::RotateRight4;
    if (diff == 5 || diff == 6) return ActionType::RotateLeft4;
    if (diff == 7) return ActionType::RotateLeft8;
    return ActionType::None;
}

std::vector<Direction::Value> Algorithm1::computeBFS(const Board& board, const Tank& self, const Tank& enemy) {
    struct Node {
        Position pos;
        std::vector<Direction::Value> path;
    };

    int width = board.getWidth(), height = board.getHeight();
    std::queue<Node> q;
    std::set<std::pair<int, int>> visited;
    q.push({self.getPosition(), {}});

    std::vector<Direction::Value> bestPath;
    int shortestLength = std::numeric_limits<int>::max();

    while (!q.empty()) {
        Node current = q.front(); q.pop();
        auto key = std::make_pair(current.pos.x, current.pos.y);
        if (visited.count(key)) continue;
        visited.insert(key);

        Tank fakeTank(self.getSymbol(), self.getAmmo(), self.getDirection(), current.pos,
                      self.getShootingStatus(), self.getBackwardStatus());

        if (canShoot(fakeTank, enemy, board)) {
            if (current.path.size() < shortestLength) {
                bestPath = current.path;
                shortestLength = current.path.size();
            }
            continue; // ממשיך לבדוק עוד אופציות
        }

        for (int i = 0; i < 8; ++i) {
            Direction::Value dir = static_cast<Direction::Value>(i);
            Position next = current.pos + Direction(dir).toVector();

            if (next.x < 0 || next.x >= width || next.y < 0 || next.y >= height)
                continue;

            if (visited.count({next.x, next.y})) continue;

            const CellSlot& slot = board.getSlot(next.x, next.y);
            if (slot.getMine()) continue; // אל תעבור דרך מוקש

            std::vector<Direction::Value> newPath = current.path;
            newPath.push_back(dir);
            q.push({next, newPath});
        }
    }

    return bestPath; // עשוי להיות ריק אם אין שום מסלול לירי
}


Action Algorithm1::nextAction(const Board& board, const Tank& self, const Tank& enemy) {
    int width = board.getWidth(), height = board.getHeight();

    // אם אפשר לירות – יורה מיד
    if (canShoot(self, enemy, board) && self.getShootingStatus() == 0 && self.getAmmo() > 0) {
        return Action(ActionType::Shoot);
    }
      if (self.getAmmo() == 0) {
        std::cout << " tamk1 no ammo.\n";
        return Action(ActionType::None);
    }
    


    // אם אין מסלול או האויב זז – מחשב מסלול חדש
    if (currentPath.empty() || !(enemy.getPosition() == lastEnemyPos)) {
        currentPath = computeBFS(board, self, enemy);
        lastEnemyPos = enemy.getPosition();
    }

    if (currentPath.empty()) {
        return Action(ActionType::None);
    }

    Direction::Value targetDir = currentPath.front();

    if (self.getDirection().getDirection() == targetDir) {
        Position nextPos = self.getPosition() + self.getDirection().toVector();

        // ✅ בדיקת גבולות לפני getSlot
        if (nextPos.x < 0 || nextPos.x >= width || nextPos.y < 0 || nextPos.y >= height) {
            return Action(ActionType::None);
        }

        const CellSlot& slot = board.getSlot(nextPos.x, nextPos.y);

        if (slot.getWall()) {
            if (self.getShootingStatus() == 0 && self.getAmmo() > 0) {
                return Action(ActionType::Shoot);
            }
            return Action(ActionType::None);
        }

        currentPath.erase(currentPath.begin());
        return Action(ActionType::MoveForward);
    }

    return Action(rotateTowards(self.getDirection().getDirection(), targetDir));
}
