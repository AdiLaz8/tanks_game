#include "Algorithm1.h"
#include <queue>
#include <set>
#include <iostream>
#include "Logger.h"

bool triedPathWithoutSuccess = false; // a boolean to indicate if we are stuck in the current BFS path and need to recompute the path
Position lastEnemyPos = {-1, -1}; // invalid value for the start
std::vector<Direction::Value> currentPath; // the current path we got from the BFS computation

std::vector<Direction::Value> Algorithm1::computeBFS(const Board& board, const Tank& self, const Tank& enemy) {
    // a struct for nodes to help us with the BFS computation
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
        // checking if the next step in the queue can lead to shooting the enemy directly
        Tank fakeTank(self.getSymbol(), self.getAmmo(), self.getDirection(), current.pos,
                      self.getShootingStatus(), self.getBackwardStatus()); // a fake tank to do the testing on

        if (canShoot(fakeTank, enemy, board)) {
            if (current.path.size() < shortestLength) {
                bestPath = current.path;
                shortestLength = current.path.size();
            }
            continue;
        }
        // if the next step can't lead to a shooting position
        for (int i = 0; i < 8; ++i) {
            Direction::Value dir = static_cast<Direction::Value>(i);
            Position next = current.pos + Direction(dir).toVector();

            if (next.x < 0 || next.x >= width || next.y < 0 || next.y >= height)
                continue;

            if (visited.count({next.x, next.y})) continue;
            // if there's a mine, don't go to it
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
    // if threteand by shells, escape as self defense
    if (isThreatenedByShells(board, self.getPosition())) {
        Logger::debug("Player 1: Threatened By Shells");
        return moveIfThreatened(board, self);
    }
    // if there is no ammo left, he can't be agressive so he does nothing if he is not threteand
    if (self.getAmmo() == 0) {
        Logger::debug("Player 1: No ammo");
        return Action(ActionRequest::DoNothing);
    }
    Logger::debug("Player 1: Checking shooting condition - canShoot: " +
              std::string(canShoot(self, enemy, board) ? "true" : "false") +
              ", shootingStatus: " + std::to_string(self.getShootingStatus()));
    // if he has a direct path to shoot and also is not in cooldown, he shoots
    if (canShoot(self, enemy, board) && self.getShootingStatus() == 0) {
        Logger::debug("Player 1: Enemy in direction " + std::to_string(self.getDirection().getDirection()) + ". Shooting now.");
        triedPathWithoutSuccess = false;
        return Action(ActionRequest::Shoot);
    }
    // checks if he can rotate towards a shooting position
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

    bool enemyMoved = !(enemy.getPosition() == lastEnemyPos); // checks if the enemy tank moved in the last step
    bool shouldComputeBFS = currentPath.empty() || enemyMoved || triedPathWithoutSuccess; // checks if he doesn't have a BFS computed or the enemy moved, or the current path in the BFS blocked
    // computes BFS again - doesn't do it every turn - only when needed
    if (shouldComputeBFS) {
        std::string reason = currentPath.empty() ? "empty" : (enemyMoved ? "enemy moved" : "triedPathWithoutSuccess");
        Logger::debug("Player 1: Computing BFS (reason: " + reason + ")");
        currentPath = computeBFS(board, self, enemy);
        triedPathWithoutSuccess = currentPath.empty();
        lastEnemyPos = enemy.getPosition();
    }
    // if there is no path for the enemy tank, he is stuck and can't move he shoots if possible or rotating to the right if not
    if (currentPath.empty()) {
        if (!canShoot(self, enemy, board)) {
            Logger::debug("Player 1: Stuck, rotating randomly");
            triedPathWithoutSuccess = true;

            // if (self.getShootingStatus() == 0 && self.getAmmo() > 0) {
            //     Logger::debug("Player 1: Shooting randomly due to stuck state");
            //     return Action(ActionRequest::Shoot);
            // }

            return Action(ActionRequest::RotateRight45);
        }

        Logger::debug("Player 1: No path but can shoot directly");
        triedPathWithoutSuccess = false;
        return Action(ActionRequest::Shoot);
    }
    // if the BFS is computed correctly, do the next step in the BFS path
    Direction::Value targetDir = currentPath.front();
    if (self.getDirection().getDirection() == targetDir) {
        Position nextPos = self.getPosition() + Direction(targetDir).toVector();

        const CellSlot& slot = board.getSlot(nextPos.x, nextPos.y);
        // won't go to a cell that has a mine
        if (slot.getMine()) {
            Logger::debug("Player 1: Mine ahead – aborting move and resetting path");
            currentPath.clear();
            triedPathWithoutSuccess = true;
            return Action(ActionRequest::DoNothing);
        }
        // if he has a wall in his BFS path, he shoots it in order to take it down
        if (slot.getWall()) {
            if (self.getShootingStatus() == 0 && self.getAmmo() > 0) {
                Logger::debug("Player 1: Wall ahead – shooting it");
                triedPathWithoutSuccess = false;
                return Action(ActionRequest::Shoot);
            }
            return Action(ActionRequest::DoNothing);
        }
        // after doing the path, remove it from the current path steps and move forward
        currentPath.erase(currentPath.begin());
        Logger::debug("Player 1: Moving forward to (" + std::to_string(nextPos.x) + "," + std::to_string(nextPos.y) + ")");
        triedPathWithoutSuccess = false;
        return Action(ActionRequest::MoveForward);
    }
    // if he isn't able to do anything, just rotate and maybe it will help in the next steps
    Logger::debug("Player 1: Rotating from " + std::to_string(self.getDirection().getDirection()) + " to " + std::to_string(targetDir));
    triedPathWithoutSuccess = false;
    return Action(rotateTowards(self.getDirection().getDirection(), targetDir));
}
