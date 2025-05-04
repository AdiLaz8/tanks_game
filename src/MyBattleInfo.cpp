#include "MyBattleInfo.h"

void MyBattleInfo::reset() {
    fullView.clear();
    directionalView.clear();
    shellPositions.clear();
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            localView[i][j] = ' ';
    selfPositionSet = false;
}
MyBattleInfo::MyBattleInfo() {
    reset();
}


void MyBattleInfo::addObject(const Position& pos, char symbol, int playerId, int boardRows, int boardCols) {
    if (symbol == '*')
        shellPositions.push_back(pos);

    if (symbol == '%') {
        selfPosition = pos;
        selfPositionSet = true;
    }

    if (playerId == 1)
        fullView.emplace_back(pos, symbol);

    if (!selfPositionSet)
        return;

    // localView
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            Position neighbor = {selfPosition.getx() + dx, selfPosition.gety() + dy};
            if (neighbor.getx() >= 0 && neighbor.getx() < boardCols &&
                neighbor.gety() >= 0 && neighbor.gety() < boardRows) {
                if (neighbor == pos) {
                    localView[dx + 1][dy + 1] = symbol;
                }
            }
        }
    }

    // directionalView
    Position delta = selfDirection.toVector();
    Position current = selfPosition + delta;
    while (current.getx() >= 0 && current.getx() < boardCols &&
           current.gety() >= 0 && current.gety() < boardRows) {
        if (current == pos)
            directionalView.emplace_back(current, symbol);
        current = current + delta;
    }
}

const std::vector<std::pair<Position, char>>& MyBattleInfo::getFullView() const {
    return fullView;
}

const char (&MyBattleInfo::getLocalView() const)[3][3] {
    return localView;
}

const std::vector<std::pair<Position, char>>& MyBattleInfo::getDirectionalView() const {
    return directionalView;
}

const std::vector<Position>& MyBattleInfo::getShellPositions() const {
    return shellPositions;
}

Position MyBattleInfo::getSelfPosition() const {
    return selfPosition;
}

Direction MyBattleInfo::getSelfDirection() const {
    return selfDirection;
}

void MyBattleInfo::setSelfDirection(Direction d) {
    selfDirection = d;
}

void MyBattleInfo::setSelfPosition(Position p) {
    selfPosition = p;
}