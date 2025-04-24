#include "Algorithm1.h"
#include <queue>
#include <set>
#include <iostream>
#include "Logger.h"

bool triedPathWithoutSuccess = false;
Position lastEnemyPos = {-1, -1}; // ערך לא חוקי להתחלה
std::vector<Direction::Value> currentPath;

std::vector<Direction::Value> Algorithm1::computeBFS(const Board& board, const Tank& self, const Tank& enemy) {
    struct Node {
        Position pos;
        std::vector<Direction::Value> path;
    };
    Logger::debug("Player 1 - BFS: starting BFS from position (" + std::to_string(self.getPosition().x) + "," + std::to_string(self.getPosition().y) + ")");

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
            continue;
        }

        for (int i = 0; i < 8; ++i) {
            Direction::Value dir = static_cast<Direction::Value>(i);
            Position next = current.pos + Direction(dir).toVector();

            if (next.x < 0 || next.x >= width || next.y < 0 || next.y >= height)
                continue;

            if (visited.count({next.x, next.y})) continue;

            const CellSlot& slot = board.getSlot(next.x, next.y);
            if (slot.getMine()) continue;

            std::vector<Direction::Value> newPath = current.path;
            newPath.push_back(dir);
            q.push({next, newPath});
        }
    }

    return bestPath;
}

Action Algorithm1::nextAction(const Board& board, const Tank& self, const Tank& enemy) {
    if (isThreatenedByShells(board, self.getPosition())) {
        Logger::debug("Player 1: Threatened By Shells");
        return moveIfThreatened(board, self);
    }

    if (self.getAmmo() == 0) {
        Logger::debug("Player 1: No ammo");
        return Action(ActionType::None);
    }
    Logger::debug("Player 1: Checking shooting condition - canShoot: " +
              std::string(canShoot(self, enemy, board) ? "true" : "false") +
              ", shootingStatus: " + std::to_string(self.getShootingStatus()));

    if (canShoot(self, enemy, board) && self.getShootingStatus() == 0) {
        Logger::debug("Player 1: Enemy in direction " + std::to_string(self.getDirection().getDirection()) + ". Shooting now.");
        triedPathWithoutSuccess = false;
        return Action(ActionType::Shoot);
    }

    for (int dir = 0; dir < 8; ++dir) {
        if (self.getDirection().getDirection() == dir) continue;
        Direction d = Direction(static_cast<Direction::Value>(dir));
        Tank fake(self.getSymbol(), self.getAmmo(), d, self.getPosition(), self.getShootingStatus(), self.getBackwardStatus());

        if (canShoot(fake, enemy, board)) {
            Logger::debug("Player 1: Enemy in direction " + std::to_string(dir) + ", turning toward it.");
            currentPath.clear();
            triedPathWithoutSuccess = false;
            return Action(rotateTowards(self.getDirection().getDirection(), d.getDirection()));
        }
    }

    bool enemyMoved = !(enemy.getPosition() == lastEnemyPos);
    bool shouldComputeBFS = currentPath.empty() || enemyMoved || triedPathWithoutSuccess;

    if (shouldComputeBFS) {
        std::string reason = currentPath.empty() ? "empty" : (enemyMoved ? "enemy moved" : "triedPathWithoutSuccess");
        Logger::debug("Player 1: Computing BFS (reason: " + reason + ")");
        currentPath = computeBFS(board, self, enemy);
        triedPathWithoutSuccess = currentPath.empty();
        lastEnemyPos = enemy.getPosition();
    }

    if (currentPath.empty()) {
        if (!canShoot(self, enemy, board)) {
            Logger::debug("Player 1: Stuck, rotating randomly");
            triedPathWithoutSuccess = true;

            if (self.getShootingStatus() == 0 && self.getAmmo() > 0) {
                Logger::debug("Player 1: Shooting randomly due to stuck state");
                return Action(ActionType::Shoot);
            }

            return Action(ActionType::RotateRight8);
        }

        Logger::debug("Player 1: No path but can shoot directly");
        triedPathWithoutSuccess = false;
        return Action(ActionType::Shoot);
    }

    Direction::Value targetDir = currentPath.front();
    if (self.getDirection().getDirection() == targetDir) {
        Position nextPos = self.getPosition() + Direction(targetDir).toVector();

        const CellSlot& slot = board.getSlot(nextPos.x, nextPos.y);

        if (slot.getMine()) {
            Logger::debug("Player 1: Mine ahead – aborting move and resetting path");
            currentPath.clear();
            triedPathWithoutSuccess = true;
            return Action(ActionType::None);
        }

        if (slot.getWall()) {
            if (self.getShootingStatus() == 0 && self.getAmmo() > 0) {
                Logger::debug("Player 1: Wall ahead – shooting it");
                triedPathWithoutSuccess = false;
                return Action(ActionType::Shoot);
            }
            return Action(ActionType::None);
        }

        currentPath.erase(currentPath.begin());
        Logger::debug("Player 1: Moving forward to (" + std::to_string(nextPos.x) + "," + std::to_string(nextPos.y) + ")");
        triedPathWithoutSuccess = false;
        return Action(ActionType::MoveForward);
    }

    Logger::debug("Player 1: Rotating from " + std::to_string(self.getDirection().getDirection()) + " to " + std::to_string(targetDir));
    triedPathWithoutSuccess = false;
    return Action(rotateTowards(self.getDirection().getDirection(), targetDir));
}
