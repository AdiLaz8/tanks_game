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
    char selfSymbol() const { return playerId == 1 ? '1' : '2'; }
    char enemySymbol() const { return playerId == 1 ? '2' : '1'; }
    Direction::Value getDirectionTo(const Position& from, const Position& to) const;
    ActionRequest rotateTowards(Direction::Value current, Direction::Value desired);
    bool canShootInDirection() const;
    bool canShootInDirection(Direction dir) const;
    bool isThreatenedByShells() const;
    Action moveIfThreatened();
    int getAmmo() const;
    int getShootingStatus() const;
    Direction getTankDirection() const;
    Position getTankPosition() const;
    void decreaseShooting();
    int getTankId() const;
    void updateBattleInfo(BattleInfo& info) override;



public:
    MyTankAlgorithm(int playerIndex, int tankIndex);
    virtual ActionRequest getAction() override = 0;
    int getBackwardStatus() const;
    void decreaseBackward();
    void setBackward(int i);
};

#endif 