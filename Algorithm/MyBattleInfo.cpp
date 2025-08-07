#include "MyBattleInfo.h"
#include <set>

using namespace UserCommon_318772340_206580102;

namespace Algorithm_318772340_206580102 {

MyBattleInfo::MyBattleInfo(size_t rows, size_t cols, int numShells)
    : boardRows(rows), boardCols(cols), numShells(numShells) {
    reset();
}

void MyBattleInfo::reset() {
    fullView.clear();
    shellPositions.clear();
    selfPositionSet = false;
}

void MyBattleInfo::addObject(const Position& pos, char symbol, int playerId, int boardRows, int boardCols) {
    (void)playerId;
    (void)boardRows;
    (void)boardCols;
    if (symbol == '*')
        shellPositions.push_back(pos);

    if (symbol == '%') {
        selfPosition = pos;
        selfPositionSet = true;
    }
    fullView.emplace_back(pos, symbol);
}

const std::vector<std::pair<Position, char>>& MyBattleInfo::getFullView() const {
    return fullView;
}

const std::vector<Position>& MyBattleInfo::getShellPositions() const {
    return shellPositions;
}

Position MyBattleInfo::getSelfPosition() const {
    return selfPosition;
}

void MyBattleInfo::setSelfPosition(Position p) {
    selfPosition = p;
}

int MyBattleInfo::getInitialShells() const {
    return numShells;
}

size_t MyBattleInfo::getWidth() const {
    return boardCols;
}

size_t MyBattleInfo::getHeight() const {
    return boardRows;
}

} // namespace Algorithm_318772340_206580102
