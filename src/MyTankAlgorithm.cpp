#include "MyTankAlgorithm.h"
#include <algorithm>

MyTankAlgorithm::MyTankAlgorithm(int playerIndex, int tankIndex)
    : playerId(playerIndex), tankId(tankIndex), dir((playerIndex==1)? Direction::L : Direction::R) {}

void MyTankAlgorithm::updateBattleInfo(BattleInfo& info) {
    currentInfo = dynamic_cast<MyBattleInfo*>(&info);
    if (turnCounterSinceInfo == -1){
        boardWidth = currentInfo->getWidth();
        boardHeight = currentInfo->getHeight();
    }
    turnCounterSinceInfo = 0;
}

int MyTankAlgorithm::getTurnsSinceBattleInfo() const {
    return turnCounterSinceInfo;
}

void MyTankAlgorithm::incrementTurnCounter() {
    turnCounterSinceInfo++;
}

bool MyTankAlgorithm::canShootInDirection() const {
    const MyBattleInfo* info = dynamic_cast<MyBattleInfo*>(currentInfo);
    if (!info) return false;

    const auto& dirView = info->getDirectionalView();
    for (const auto& [pos, symbol] : dirView) {
        if (symbol == '%') return false; // don't shoot if we see ourselves
        else if ((playerId == 1 && symbol == '1') || (playerId == 2 && symbol == '2')) return false; // friendly fire
        else if ((playerId == 1 && symbol == '2') || (playerId == 2 && symbol == '1')) return true; // enemy
    }
    return false;
}

bool MyTankAlgorithm::isThreatenedByShells() const {
    if (!currentInfo) return false;

    const auto& local = currentInfo->getLocalView();
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j)
            if (local[i][j] == '*') return true;

    return false;
}


Action MyTankAlgorithm::moveIfThreatened()  {
if (!currentInfo) return Action(ActionRequest::GetBattleInfo);

Position selfPos = currentInfo->getSelfPosition();
Direction selfDir = dir;
Position forward = selfPos + selfDir.toVector();

forward.setx((forward.getx() + boardWidth) % boardWidth);
forward.sety((forward.gety() + boardHeight) % boardHeight);

const char (&local)[5][5] = currentInfo->getLocalView();
int dx = forward.getx() - selfPos.getx();
int dy = forward.gety() - selfPos.gety();

// תיקון לדלתא בלוקאל (במקרה של wraparound)
if (dx > 1) dx = -1;
if (dx < -1) dx = 1;
if (dy > 1) dy = -1;
if (dy < -1) dy = 1;

if (std::abs(dx) <= 1 && std::abs(dy) <= 1 && local[dx + 1][dy + 1] == ' ') {
    return Action(ActionRequest::MoveForward);
}

// חיפוש תא פנוי בכיוונים שהם ב־45 או 90 מעלות מהכיוון הנוכחי
Direction::Value currDir = selfDir.getDirection();
int currIndex = static_cast<int>(currDir);
std::vector<int> offsets = {1, 2, -1, -2};

for (int offset : offsets) {
    int tryIndex = (currIndex + offset + 8) % 8;
    Direction::Value tryDir = static_cast<Direction::Value>(tryIndex);
    Position delta = Direction(tryDir).toVector();
    Position target = selfPos + delta;
    target.setx((target.getx() + boardWidth) % boardWidth);
    target.sety((target.gety() + boardHeight) % boardHeight);

    int dxTry = target.getx() - selfPos.getx();
    int dyTry = target.gety() - selfPos.gety();

    if (dxTry > 1) dxTry = -1;
    if (dxTry < -1) dxTry = 1;
    if (dyTry > 1) dyTry = -1;
    if (dyTry < -1) dyTry = 1;

    if (std::abs(dxTry) <= 1 && std::abs(dyTry) <= 1 && local[dxTry + 1][dyTry + 1] == ' ') {
        moveAfterRotate=true;
        return Action(rotateTowards(currDir, tryDir));
    }
}


return Action(ActionRequest::DoNothing);
}

ActionRequest MyTankAlgorithm::rotateTowards(Direction::Value current, Direction::Value desired) const {
    int diff = (static_cast<int>(desired) - static_cast<int>(current) + 8) % 8;
    if (diff == 0) return ActionRequest::DoNothing;
    if (diff == 1) return ActionRequest::RotateRight45;
    if (diff == 2) return ActionRequest::RotateRight90;
    if (diff == 3) return ActionRequest::RotateRight90;
    if (diff == 4) return ActionRequest::RotateRight90;
    if (diff == 5) return ActionRequest::RotateLeft90;
    if (diff == 6) return ActionRequest::RotateLeft90;
    if (diff == 7) return ActionRequest::RotateLeft45;
    return ActionRequest::DoNothing;
}
Direction::Value MyTankAlgorithm::getDirectionTo(const Position& from, const Position& to) const {
    int dx = ((to.getx() - from.getx() + boardWidth) % boardWidth + boardWidth / 2) % boardWidth - boardWidth / 2;
    int dy = ((to.gety() - from.gety() + boardHeight) % boardHeight + boardHeight / 2) % boardHeight - boardHeight / 2;

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
int  MyTankAlgorithm::getTankId()const{
    return tankId;
}
Direction MyTankAlgorithm::getTankDirection()const{
    return dir;
}