// Algo1.cpp
#include "Algo1.h"
#include <cmath>
#include <unordered_map>
#include <set>

void Algo1::updateBattleInfo(BattleInfo& info) {
    currentInfo = dynamic_cast<MyBattleInfo*>(&info);
    if(!currentInfo) return;
    boardWidth = currentInfo->getWidth();
    boardHeight = currentInfo->getHeight();
    turnCounterSinceInfo = 0;

    const auto& fullView = currentInfo->getFullView();
    Position self = currentInfo->getSelfPosition();
    char enemySymbol = (playerId == 1 ? '2' : '1');

    int minDist = boardWidth * boardHeight;
    for (const auto& [pos, symbol] : fullView) {
        if (symbol == enemySymbol) {
            int dx = std::min(
                (self.getx() - pos.getx() + boardWidth) % boardWidth,
                (pos.getx() - self.getx() + boardWidth) % boardWidth
            );
            int dy = std::min(
                (self.gety() - pos.gety() + boardHeight) % boardHeight,
                (pos.gety() - self.gety() + boardHeight) % boardHeight
            );
            int dist = dx + dy;
            if (dist < minDist) {
                minDist = dist;
                targetPos = pos;
            }
        }
    }
    computeShootingPath();
}

ActionRequest Algo1::getAction() {
    // first turn
    if (turnCounterSinceInfo == -1)
        return ActionRequest::GetBattleInfo;

    if (moveAfterRotate) {
        moveAfterRotate = false;
        needsNewInfo = true;
        turnCounterSinceInfo++;
        return ActionRequest::MoveForward;
    }

    if (isThreatenedByShells()) {
        currentPath.clear();
        turnCounterSinceInfo++;
        return moveIfThreatened().getType();
    }

    if (canShootInDirection()){
        turnCounterSinceInfo++;
        return ActionRequest::Shoot;
    }

    if (needsNewBattleInfo()){
        chasing = false;
        turnCounterSinceInfo = 0;
        return ActionRequest::GetBattleInfo;
    }

    if (!currentPath.empty()) {
        Direction::Value nextDir = currentPath.front();
        if (nextDir == dir.getDirection()){
            Position selfPos = currentInfo->getSelfPosition();
            Position nextPos = selfPos + Direction(nextDir).toVector();
            nextPos.setx((nextPos.getx() + boardWidth) % boardWidth);
            nextPos.sety((nextPos.gety() + boardHeight) % boardHeight);
             // בדיקה אם יש קיר במיקום הבא
            auto it = std::find_if(fullView.begin(), fullView.end(),
                           [&](const std::pair<Position, char>& cell) {
                               return cell.first == nextPos;
                           });
            if (it != fullView.end() && it->second == '#') {
                return ActionRequest::Shoot;
            }
            if (it != fullView.end() && it->second == '@') {
                currentPath.empty();
                chasing = false;
                turnCounterSinceInfo = 0;
                return ActionRequest::GetBattleInfo;
            }
            currentPath.erase(currentPath.begin());
            turnCounterSinceInfo++;
            return ActionRequest::MoveForward;
        }
        else{
            turnCounterSinceInfo++;
            return rotateTowards(dir.getDirection(), nextDir);
        }   
    }
    turnCounterSinceInfo = 0;
    chasing = false;
    return ActionRequest::GetBattleInfo;
}

bool Algo1::needsNewBattleInfo() const {
    return needsNewInfo || turnCounterSinceInfo >= 10 || (chasing && currentPath.empty());
}

void Algo1::computeShootingPath() {
    currentPath = computeBFS(currentInfo->getSelfPosition(), targetPos,
                             currentInfo->getFullView(), boardWidth, boardHeight);
    chasing = true;
}

std::vector<Direction::Value> Algo1::computeBFS(const Position& from, const Position& to,
                                                const std::vector<std::pair<Position, char>>& fullView,
                                                size_t width, size_t height) {
    std::set<Position> blocked;
    char friendSymbol = (playerId == 1 ? '1' : '2');
    char enemySymbol = (playerId == 1 ? '2' : '1');

    for (const auto& [pos, symbol] : fullView) {
        if (symbol == '@' || symbol == friendSymbol)
            blocked.insert(pos);
    }

    std::queue<std::pair<Position, std::vector<Direction::Value>>> q;
    std::set<Position> visited;
    q.push({from, {}});
    visited.insert(from);

    while (!q.empty()) {
        auto [current, path] = q.front();
        q.pop();

        for (int i = 0; i < 8; ++i) {
            Direction::Value dirVal = static_cast<Direction::Value>(i);
            Position delta = Direction(dirVal).toVector();
            Position next = current + delta;
            next.setx((next.getx() + width) % width);
            next.sety((next.gety() + height) % height);

            if (visited.count(next) || blocked.count(next))
                continue;

            std::vector<Direction::Value> newPath = path;
            newPath.push_back(dirVal);

            // בדיקה האם מנקודה זו יש קו ירי ישיר על האויב
            Position step = next;
            Direction shootingDir(dirVal);
            for (size_t s = 0; s < std::max(width, height); ++s) {
                step.setx((step.getx() + width) % width);
                step.sety((step.gety() + height) % height);
                step = step + shootingDir.toVector();
                if (step == from)
                    break;

                for (const auto& [pos, symbol] : fullView) {
                    if (pos == step && symbol == enemySymbol)
                        return newPath;
                }
            }

            q.push({next, newPath});
            visited.insert(next);
        }
    }

    return {};
}