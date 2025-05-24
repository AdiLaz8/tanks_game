// // Algo1.cpp
// #include "Algo1.h"
// #include <cmath>
// #include <unordered_map>
// #include <set>

// void Algo1::updateBattleInfo(BattleInfo& info) {
//     currentInfo = dynamic_cast<MyBattleInfo*>(&info);
//     if(!currentInfo) return;
//     boardWidth = currentInfo->getWidth();
//     boardHeight = currentInfo->getHeight();
//     turnCounterSinceInfo = 0;

//     const auto& fullView = currentInfo->getFullView();
//     Position self = currentInfo->getSelfPosition();
//     char enemySymbol = (playerId == 1 ? '2' : '1');

//     int minDist = boardWidth * boardHeight;
//     for (const auto& [pos, symbol] : fullView) {
//         if (symbol == enemySymbol) {
//             int dx = std::min(
//                 (self.getx() - pos.getx() + boardWidth) % boardWidth,
//                 (pos.getx() - self.getx() + boardWidth) % boardWidth
//             );
//             int dy = std::min(
//                 (self.gety() - pos.gety() + boardHeight) % boardHeight,
//                 (pos.gety() - self.gety() + boardHeight) % boardHeight
//             );
//             int dist = dx + dy;
//             if (dist < minDist) {
//                 minDist = dist;
//                 targetPos = pos;
//             }
//         }
//     }
//     computeShootingPath();
// }

// ActionRequest Algo1::getAction() {
//     // first turn
//     if (turnCounterSinceInfo == -1)
//         return ActionRequest::GetBattleInfo;
// // 
//     if (moveAfterRotate) {
//         moveAfterRotate = false;
//         needsNewInfo = true;
//         turnCounterSinceInfo++;
//         return ActionRequest::MoveForward;
//     }

//     if (isThreatenedByShells()) {
//         currentPath.clear();
//         turnCounterSinceInfo++;
//         return moveIfThreatened().getType();
//     }

//     if (canShootInDirection()){
//         turnCounterSinceInfo++;
//         return ActionRequest::Shoot;
//     }

//     if (needsNewBattleInfo()){
//         chasing = false;
//         turnCounterSinceInfo = 0;
//         return ActionRequest::GetBattleInfo;
//     }

//     if (!currentPath.empty()) {
//         Direction::Value nextDir = currentPath.front();
//         if (nextDir == dir.getDirection()){
//             Position selfPos = currentInfo->getSelfPosition();
//             Position nextPos = selfPos + Direction(nextDir).toVector();
//             nextPos.setx((nextPos.getx() + boardWidth) % boardWidth);
//             nextPos.sety((nextPos.gety() + boardHeight) % boardHeight);
//              // בדיקה אם יש קיר במיקום הבא
//             auto it = std::find_if(fullView.begin(), fullView.end(),
//                            [&](const std::pair<Position, char>& cell) {
//                                return cell.first == nextPos;
//                            });
//             if (it != fullView.end() && it->second == '#') {
//                 return ActionRequest::Shoot;
//             }
//             if (it != fullView.end() && it->second == '@') {
//                 currentPath.empty();
//                 chasing = false;
//                 turnCounterSinceInfo = 0;
//                 return ActionRequest::GetBattleInfo;
//             }
//             currentPath.erase(currentPath.begin());
//             turnCounterSinceInfo++;
//             return ActionRequest::MoveForward;
//         }
//         else{
//             turnCounterSinceInfo++;
//             return rotateTowards(dir.getDirection(), nextDir);
//         }   
//     }
//     turnCounterSinceInfo = 0;
//     chasing = false;
//     return ActionRequest::GetBattleInfo;
// }

// bool Algo1::needsNewBattleInfo() const {
//     return needsNewInfo || turnCounterSinceInfo >= 10 || (chasing && currentPath.empty());
// }

// void Algo1::computeShootingPath() {
//     currentPath = computeBFS(currentInfo->getSelfPosition(), targetPos,
//                              currentInfo->getFullView(), boardWidth, boardHeight);
//     chasing = true;
// }

// std::vector<Direction::Value> Algo1::computeBFS(const Position& from, const Position& to,
//                                                 const std::vector<std::pair<Position, char>>& fullView,
//                                                 size_t width, size_t height) {
//     std::set<Position> blocked;
//     char friendSymbol = (playerId == 1 ? '1' : '2');
//     char enemySymbol = (playerId == 1 ? '2' : '1');

//     for (const auto& [pos, symbol] : fullView) {
//         if (symbol == '@' || symbol == friendSymbol)
//             blocked.insert(pos);
//     }

//     std::queue<std::pair<Position, std::vector<Direction::Value>>> q;
//     std::set<Position> visited;
//     q.push({from, {}});
//     visited.insert(from);

//     while (!q.empty()) {
//         auto [current, path] = q.front();
//         q.pop();

//         for (int i = 0; i < 8; ++i) {
//             Direction::Value dirVal = static_cast<Direction::Value>(i);
//             Position delta = Direction(dirVal).toVector();
//             Position next = current + delta;
//             next.setx((next.getx() + width) % width);
//             next.sety((next.gety() + height) % height);

//             if (visited.count(next) || blocked.count(next))
//                 continue;

//             std::vector<Direction::Value> newPath = path;
//             newPath.push_back(dirVal);

//             // בדיקה האם מנקודה זו יש קו ירי ישיר על האויב
//             Position step = next;
//             Direction shootingDir(dirVal);
//             for (size_t s = 0; s < std::max(width, height); ++s) {
//                 step.setx((step.getx() + width) % width);
//                 step.sety((step.gety() + height) % height);
//                 step = step + shootingDir.toVector();
//                 if (step == from)
//                     break;

//                 for (const auto& [pos, symbol] : fullView) {
//                     if (pos == step && symbol == enemySymbol)
//                         return newPath;
//                 }
//             }

//             q.push({next, newPath});
//             visited.insert(next);
//         }
//     }

//     return {};
// }

#include "Algo1.h"
#include <cmath>
#include <set>
#include <algorithm>
#include "MyBattleInfo.h"


void Algo1::updateBattleInfo(BattleInfo& info) {
    auto& myInfo = dynamic_cast<MyBattleInfo&>(info);
    if (turnCounterSinceInfo == -1) {
        boardWidth = myInfo.getWidth();
        boardHeight = myInfo.getHeight();
        ammo = myInfo.getInitialShells();
    }

    needsNewInfo = false;
    turnCounterSinceInfo = 0;
    fullView = myInfo.getFullView();
    selfPosition = myInfo.getSelfPosition(); // ✅ זה השורה החשובה
    char enemySymbol = (playerId == 1 ? '2' : '1');

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

    computeShootingPath();
}


ActionRequest Algo1::getAction() {
    if (turnCounterSinceInfo == -1){
        return ActionRequest::GetBattleInfo;}
    turnCounterSinceInfo++;


    if(getShootingStatus()>0){
        shootingStatus--;
    }
    

    if (moveAfterRotate) {
        moveAfterRotate = false;
        needsNewInfo = true;
        turnCounterSinceInfo++;
        Position newPos = selfPosition + direction.toVector();
        newPos.setx((newPos.getx() + boardWidth) % boardWidth);
        newPos.sety((newPos.gety() + boardHeight) % boardHeight);
        selfPosition = newPos;
        return ActionRequest::MoveForward;
    }

    if (isThreatenedByShells()) {
        currentPath.clear();
        turnCounterSinceInfo++;
        return moveIfThreatened().getType();
    }

    if (canShootInDirection()) {
        if (ammo > 0 && getShootingStatus()==0) {
            shootingStatus=5;
            ammo--;
            turnCounterSinceInfo++;
            needsNewInfo = true;
            return ActionRequest::Shoot;
        }
    }
    for (int i = 0; i < 8; ++i) {
        if (i == static_cast<int>(direction.getDirection())) continue;

        Direction tryDir(static_cast<Direction::Value>(i));
        Direction originalDir = direction;
        direction = tryDir;

        if (canShootInDirection()) {
            direction = originalDir; // מחזירים את הכיוון המקורי כדי לא לשבש את הלוגיקה
            return rotateTowards(originalDir.getDirection(), tryDir.getDirection());
        }

        direction = originalDir; // גם אם לא הצליח, להחזיר את הכיוון המקורי
    }


    if (needsNewBattleInfo()) {
        chasing = false;
        turnCounterSinceInfo = 0;
        return ActionRequest::GetBattleInfo;
    }

    if (!currentPath.empty()) {
        Direction::Value nextDir = currentPath.front();
        if (nextDir == direction.getDirection()) {
            Position nextPos = selfPosition + Direction(nextDir).toVector();
            nextPos.setx((nextPos.getx() + boardWidth) % boardWidth);
            nextPos.sety((nextPos.gety() + boardHeight) % boardHeight);
            auto it = std::find_if(fullView.begin(), fullView.end(),
                                   [&](const auto& cell) {
                                       return cell.first == nextPos;
                                   });

        if (it != fullView.end()) {
            // ❌ אם הצעד הבא הוא מוקש או טנק (שלך או של האויב) — נחשב מסלול חדש
            if (it->second == '@' || it->second == '1' || it->second == '2') {
                currentPath.clear();
                needsNewInfo = true;
                return ActionRequest::GetBattleInfo;
            }

            // אם יש קיר – יורה
            if (it->second == '#') {
                if (ammo > 0 && getShootingStatus() == 0) {
                    shootingStatus = 4;
                    ammo--;
                    needsNewInfo = true;
                    return ActionRequest::Shoot;
                }
            }
        }

            currentPath.erase(currentPath.begin());
            selfPosition = nextPos;
            turnCounterSinceInfo++;
            return ActionRequest::MoveForward;
        } else {
            direction = Direction(nextDir);
            turnCounterSinceInfo++;
            return rotateTowards(direction.getDirection(), nextDir);
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
    currentPath = computeBFS(selfPosition, targetPos, fullView, boardWidth, boardHeight);
    chasing = true;
}

std::vector<Direction::Value> Algo1::computeBFS(const Position& from, const Position& to,
                                                const std::vector<std::pair<Position, char>>& fullView,
                                                size_t width, size_t height) {
    (void)to; // Unused parameter
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
