#pragma once

#include "TankAlgorithm.h"
#include "MyBattleInfo.h" // תיצור בהמשך את המחלקה הזו
#include "ActionRequest.h"

class MyTankAlgorithm : public TankAlgorithm {
public:
    MyTankAlgorithm(int playerIndex, int tankIndex);

    ActionRequest getAction() override;
    void updateBattleInfo(BattleInfo& info) override;
    Direction getDirection() const { return tankDirection; }
    int getTankIndex() const;
    int getPlayerIndex() const;

private:
    int playerIndex;
    int tankIndex;
    MyBattleInfo lastInfo;
    bool hasInfo = false;
    Direction tankDirection;
};
