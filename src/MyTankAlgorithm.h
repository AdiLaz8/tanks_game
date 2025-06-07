// MyTankAlgorithm.h
#ifndef MY_TANK_ALGORITHM_H
#define MY_TANK_ALGORITHM_H

#include "TankAlgorithm.h"
#include "Action.h"
#include "Direction.h"
#include "Position.h"
#include <cstddef>
#include <vector>
#include <utility>

class MyTankAlgorithm : public TankAlgorithm {
protected:
    int turnCounterSinceInfo = -1;
    int playerId;
    int tankId;
    size_t boardWidth;
    size_t boardHeight;
    std::vector<std::pair<Position, char>> fullView;
    Direction direction;
    Position selfPosition;
    int ammo = 0;
    int shootingStatus = 0;
    int backwardStatus = 0;
    std::vector<Position> minePositions;
    bool minesInitialized = false;
    bool moveAfterRotate = false;
    bool isMine(const Position& pos) const;
public:
    MyTankAlgorithm(int playerIndex, int tankIndex);

    void updateBattleInfo(BattleInfo& info) override;

    virtual ActionRequest getAction() override = 0;

    int getTankId() const;
    Direction getTankDirection() const;
    Position getTankPosition() const;
    int getAmmo() const;
    int getShootingStatus() const;
    int getBackwardStatus() const;

    void decreaseShooting();
    void decreaseBackward();

    bool isThreatenedByShells() const;
    Action moveIfThreatened();
    bool canShootInDirection() const;

    Direction::Value getDirectionTo(const Position& from, const Position& to) const;
    ActionRequest rotateTowards(Direction::Value current, Direction::Value desired);
    void setBackward(int i);
};

#endif // MY_TANK_ALGORITHM_H
