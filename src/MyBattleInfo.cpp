#include "MyBattleInfo.h"

void MyBattleInfo::setMyPosition(const Position& pos) {
    myPosition = pos;
}

Position MyBattleInfo::getMyPosition() const {
    return *myPosition;
}

bool MyBattleInfo::hasMyPosition() const {
    return myPosition.has_value();
}

void MyBattleInfo::setShellsLeft(int shells) {
    shellsLeft = shells;
}

int MyBattleInfo::getShellsLeft() const {
    return shellsLeft;
}

void MyBattleInfo::setEnemyVisible(bool visible) {
    enemyVisible = visible;
}

bool MyBattleInfo::isEnemyVisible() const {
    return enemyVisible;
}

void MyBattleInfo::clear() {
    myPosition.reset();
    shellsLeft = 0;
    enemyVisible = false;
}
