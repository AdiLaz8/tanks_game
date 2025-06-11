#include "Algo1.h"
#include <cmath>
#include <set>
#include <algorithm>
#include "MyBattleInfo.h"
#include <iostream>

void Algo1::updateBattleInfo(BattleInfo& info) {
    auto& myInfo = dynamic_cast<MyBattleInfo&>(info);
    // first turn
    if (turnCounterSinceInfo == -1) {
        boardWidth = myInfo.getWidth();
        boardHeight = myInfo.getHeight();
        ammo = myInfo.getInitialShells();
    }
    needsNewInfo = false;
    turnCounterSinceInfo = 0;
    fullView = myInfo.getFullView();
    selfPosition = myInfo.getSelfPosition(); 
    char enemySymbol = (playerId == 1 ? '2' : '1');
    //calculating closest enemy tank 
    int minDist = boardWidth * boardHeight;
    for (const auto& [pos, symbol] : fullView) {
        if (symbol == enemySymbol) {
            int dx = std::min((selfPosition.getx() - pos.getx() + boardWidth) % boardWidth,
                              (pos.getx() - selfPosition.getx() + boardWidth) % boardWidth);
            int dy = std::min((selfPosition.gety() - pos.gety() + boardHeight) % boardHeight,
                              (pos.gety() - selfPosition.gety() + boardHeight) % boardHeight);
            int dist = dx + dy;
            if (dist < minDist) {
                minDist = dist;
                targetPos = pos;
            }
        }
    }
    // initializing mines list
    if (!minesInitialized) {
        minePositions.clear();
        for (const auto& [pos, symbol] : fullView) {
            if (symbol == '@') {
                minePositions.push_back(pos);
            }
        }
        minesInitialized = true;
    }
    computeShootingPath();
}

void Algo1::computeShootingPath() {
    currentPath = computeBFS(selfPosition, targetPos, fullView, boardWidth, boardHeight);
    chasing = true;
}

std::vector<Direction::Value> Algo1::computeBFS(const Position& from, const Position& to,const std::vector<std::pair<Position, char>>& fullView,size_t width, size_t height) {
    (void)to; 
    std::set<Position> blocked;
    char friendSymbol = (playerId == 1 ? '1' : '2');
    char enemySymbol = (playerId == 1 ? '2' : '1');
    for (const auto& [pos, symbol] : fullView) {
        if (isMine(pos) || symbol == '@' || symbol == friendSymbol) // blocking mines and friendly tanks
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

ActionRequest Algo1::getShootingActionIfAvailable(){
    bool canShootNow = canShootInDirection();
    //if enemy in sight --> shoot
    if (canShootNow) {
        if (ammo > 0 && getShootingStatus()==0) {
            shootingStatus=5;
            ammo--;
            turnCounterSinceInfo++;
            needsNewInfo = true;
            check++;
            return ActionRequest::Shoot;
        }
    }
    //searching for enemy in sight in one of the directions
    for (int i = 0; i < 8; ++i) {
        if (i == static_cast<int>(direction.getDirection())) continue;
        Direction tryDir(static_cast<Direction::Value>(i));
        Direction originalDir = direction;
        direction = tryDir;
        if (canShootInDirection()) {
            direction = originalDir; 
            turnCounterSinceInfo++;
            return rotateTowards(originalDir.getDirection(), tryDir.getDirection());
        }
        direction = originalDir; 
    }
    return ActionRequest::DoNothing;
}

//if the tank is threatened (or during a chase) and rotated last turn so now it needs to move forward
ActionRequest Algo1::moveForwardAfterRotate(){
    // if it's still threatened by shells, we need to move
    if (isThreatenedByShells()) {
        currentPath.clear();
        turnCounterSinceInfo++;
        return moveIfThreatened().getType();
    }
    // if we rotated last turn, we move forward if there's no mine in front
    if (moveAfterRotate) {
            moveAfterRotate = false;
            needsNewInfo = true;
            turnCounterSinceInfo++;
            Position newPos = selfPosition + direction.toVector();
            newPos.setx((newPos.getx() + boardWidth) % boardWidth);
            newPos.sety((newPos.gety() + boardHeight) % boardHeight);
            if (!isMine(newPos)) {
                selfPosition = newPos;
                return ActionRequest::MoveForward;
            }
        }
    return ActionRequest::DoNothing;
}

// get the action from the BFS path
ActionRequest Algo1::getActionFromPath() {
    if (!currentPath.empty()) {
        Direction::Value nextDir = currentPath.front();
        if (nextDir == direction.getDirection()) {
            Position nextPos = selfPosition + Direction(nextDir).toVector();
            nextPos.setx((nextPos.getx() + boardWidth) % boardWidth);
            nextPos.sety((nextPos.gety() + boardHeight) % boardHeight);
            auto it = std::find_if(fullView.begin(), fullView.end(),[&](const auto& cell) {return cell.first == nextPos;});
            if (it != fullView.end()) {
                if (it->second == '@' || it->second == '1' || it->second == '2') {
                    currentPath.clear();
                    needsNewInfo = true;
                    turnCounterSinceInfo = 0;
                    return ActionRequest::GetBattleInfo;
                }
                if (it->second == '#') {
                    if (ammo > 0 && getShootingStatus() == 0) {
                        shootingStatus = 4;
                        ammo--;
                        needsNewInfo = true;
                        turnCounterSinceInfo++;
                        return ActionRequest::Shoot;
                    }
                }
            }
            currentPath.erase(currentPath.begin());
            if (!isMine(nextPos)) {
                selfPosition = nextPos;
                turnCounterSinceInfo++;
                return ActionRequest::MoveForward;
            } else {
                currentPath.clear();
                chasing = false;
                turnCounterSinceInfo = 0;
                return ActionRequest::GetBattleInfo;
            }
        } else {
            direction = Direction(nextDir);
            moveAfterRotate = true;
            turnCounterSinceInfo++;
            return rotateTowards(direction.getDirection(), nextDir);
        }
    }
    return ActionRequest::DoNothing; // No actions available rom the path
}

ActionRequest Algo1::getAction() {
    ActionRequest action = ActionRequest::DoNothing;
    if (turnCounterSinceInfo == -1 || turnCounterSinceInfo == 5) {
        return ActionRequest::GetBattleInfo;
    }
    if(getShootingStatus()>0){
        shootingStatus--;
    }
    action = moveForwardAfterRotate();
    if (action != ActionRequest::DoNothing) {
        return action;
    }
    if (needsNewBattleInfo()) {
        chasing = false;
        turnCounterSinceInfo = 0;
        return ActionRequest::GetBattleInfo;
    }
    action = getShootingActionIfAvailable();
    if (action != ActionRequest::DoNothing) {
        return action;
    }
    // try to get an action from the BFS path
    action = getActionFromPath();
    if (action != ActionRequest::DoNothing) {
        return action;
    }
    turnCounterSinceInfo = 0;
    chasing = false;
    return ActionRequest::GetBattleInfo;

}

// Check if the tank needs new battle info based on various conditions
bool Algo1::needsNewBattleInfo() const {
    return needsNewInfo || turnCounterSinceInfo >= 5 || (chasing && currentPath.empty());
}