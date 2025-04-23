#include "Algorithm1.h"
#include <queue>
#include <set>
#include <iostream>
#include "Logger.h"

bool triedPathWithoutSuccess = false;

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
    Logger::debug("BFS: starting BFS from position (" + std::to_string(self.getPosition().x) + "," + std::to_string(self.getPosition().y) + ")");


    int width = board.getWidth(), height = board.getHeight();
    std::queue<Node> q;
    std::set<std::pair<int, int>> visited;
    q.push({self.getPosition(), {}});

    std::vector<Direction::Value> bestPath;
    size_t shortestLength = std::numeric_limits<size_t>::max();

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

    if (self.getAmmo() == 0) {
        Logger::debug("Algorithm1: Tank has no ammo. Skipping action.");
        return Action(ActionType::None);
    }
    for (int dir = 0; dir < 8; ++dir) {
        Direction dirVal = Direction(static_cast<Direction::Value>(dir));
        Tank testTank(
            self.getSymbol(),
            self.getAmmo(),
            dirVal,
            self.getPosition(),
            self.getShootingStatus(),
            self.getBackwardStatus(),
            self.isAlive()
        );

        if (canShoot(testTank, enemy, board)) {
            if (dir == self.getDirection().getDirection()) {
                if (self.getShootingStatus() == 0) {
                    Logger::debug("Algorithm1: Enemy in direction " + std::to_string(dir) + ". Shooting now.");
                    currentPath.clear();
                    return Action(ActionType::Shoot);
                }
            } else {
                Logger::debug("Algorithm1: Enemy in direction " + std::to_string(dir) + ", turning toward it.");
                currentPath.clear();
                return Action(rotateTowards(self.getDirection().getDirection(), testTank.getDirection().getDirection()));
            }
        }
    }
    if (isThreatenedByShells(board, self.getPosition())) {
        return moveIfThreatened(board, self);
    }
    


    // אם אין מסלול או האויב זז – מחשב מסלול חדש
    if (currentPath.empty() || !(enemy.getPosition() == lastEnemyPos)|| triedPathWithoutSuccess) {
        Logger::debug("Algorithm1: Computing BFS because enemy moved or path is empty.");
        currentPath = computeBFS(board, self, enemy);
        lastEnemyPos = enemy.getPosition();
        triedPathWithoutSuccess = false;
    }

    if (currentPath.empty()) {
        if (!canShoot(self, enemy, board)) {
            Logger::debug("Algorithm1: No valid path to shooting position. Trying fallback.");
            triedPathWithoutSuccess = true;

            // ניסיון לצאת מהתקיעה:
            if (self.getShootingStatus() == 0 && self.getAmmo() > 0) {
                // אולי יירה על משהו אחר, אולי לא – עדיף מלא לעשות כלום
                Logger::debug("Algorithm1: Trying to shoot randomly due to stuck state.");
                return Action(ActionType::Shoot);
            }

            // הסתובבות רנדומלית (כדי להכניס שינוי)
            return Action(ActionType::RotateRight8);
        }
    Logger::debug("Algorithm1: No path but line of fire is available. Shooting.");
    return Action(ActionType::Shoot);
    }


    Direction::Value targetDir = currentPath.front();
    if (self.getDirection().getDirection() == targetDir) {
        Position nextPos = self.getPosition() + self.getDirection().toVector();

        if (nextPos.x < 0 || nextPos.x >= width || nextPos.y < 0 || nextPos.y >= height) {
            return Action(ActionType::None);
        }

        const CellSlot& slot = board.getSlot(nextPos.x, nextPos.y);

        if (slot.getMine()) {
            Logger::debug("Algorithm1: Next cell is a mine – recomputing BFS.");
            currentPath.clear(); // נאפס את המסלול ונחשב מחדש בתור הבא
            triedPathWithoutSuccess = true;
            return Action(ActionType::None); // אפשר גם להחזיר Rotate כדי לא לבזבז תור
        }

        if (slot.getWall()) {
            if (self.getShootingStatus() == 0 && self.getAmmo() > 0) {
                Logger::debug("Algorithm1: Wall detected ahead. Attempting to shoot it.");
                return Action(ActionType::Shoot);
            }
            return Action(ActionType::None);
        }

        currentPath.erase(currentPath.begin());
        Logger::debug("Algorithm1: Moving forward to (" +
                    std::to_string(nextPos.x) + "," +
                    std::to_string(nextPos.y) + ")");
        return Action(ActionType::MoveForward);
    }

    Logger::debug("Algorithm1: Rotating from direction " +
              std::to_string(self.getDirection().getDirection()) +
              " to " + std::to_string(targetDir));
    return Action(rotateTowards(self.getDirection().getDirection(), targetDir));
}