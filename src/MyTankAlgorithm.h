#ifndef MY_TANK_ALGORITHM_H
#define MY_TANK_ALGORITHM_H

#include "TankAlgorithm.h"
#include "MyBattleInfo.h"
#include "Board.h"
#include "Action.h"
#include <memory>

class MyTankAlgorithm : public TankAlgorithm {
protected:
    MyBattleInfo* currentInfo = nullptr;
    int turnCounterSinceInfo = -1;
    int playerId;
    int tankId;
    size_t boardWidth;
    size_t boardHeight;
    Direction dir;
    bool moveAfterRotate=false;
    

public:
    MyTankAlgorithm(int playerIndex, int tankIndex);
    void updateBattleInfo(BattleInfo& info) override;
    int getTurnsSinceBattleInfo() const;
    void incrementTurnCounter();
    int getTankId() const;
    Direction getTankDirection() const;
    bool canShootInDirection() const;
    bool isThreatenedByShells() const;
    Action moveIfThreatened() ;
    Direction::Value getDirectionTo(const Position& from, const Position& to) const;
    ActionRequest rotateTowards(Direction::Value current, Direction::Value desired) const;
    virtual ActionRequest getAction() = 0;
};

#endif // MY_TANK_ALGORITHM_H
