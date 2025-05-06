#include "MyTankAlgorithm.h"

MyTankAlgorithm::MyTankAlgorithm(int playerIndex, int tankIndex)
    : playerIndex(playerIndex), tankIndex(tankIndex),
      tankDirection(playerIndex == 1 ? Direction::L : Direction::R) {}


int MyTankAlgorithm::getTankIndex() const {
    return tankIndex;
}

int MyTankAlgorithm::getPlayerIndex() const {
    return playerIndex;
}

void MyTankAlgorithm::updateBattleInfo(BattleInfo& info) {
    // נניח ש־info הוא MyBattleInfo
    MyBattleInfo* myInfo = dynamic_cast<MyBattleInfo*>(&info);
    if (myInfo) {
        lastInfo = *myInfo; // העתקה
        hasInfo = true;
    }
}

ActionRequest MyTankAlgorithm::getAction() {
    if (!hasInfo) {
        return ActionRequest::GetBattleInfo;
    }

    // כאן תהיה הלוגיקה האמיתית בהמשך
    // לדוגמה בסיסית:
    return ActionRequest::DoNothing;
}
