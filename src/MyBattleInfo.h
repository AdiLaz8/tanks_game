#ifndef MY_BATTLE_INFO_H
#define MY_BATTLE_INFO_H

#include "BattleInfo.h"
#include "Position.h"
#include "Direction.h"
#include <vector>

class MyBattleInfo : public BattleInfo {
public:
    MyBattleInfo();
    void reset();
    void addObject(const Position& pos, char symbol, int playerId, int boardRows, int boardCols);

    const std::vector<std::pair<Position, char>>& getFullView() const;
    const char (&getLocalView() const)[3][3];
    const std::vector<std::pair<Position, char>>& getDirectionalView() const;
    const std::vector<Position>& getShellPositions() const;
    Position getSelfPosition() const;
    Direction getSelfDirection() const;
    void setSelfDirection(Direction d);

private:
    std::vector<std::pair<Position, char>> fullView;
    char localView[3][3];
    std::vector<std::pair<Position, char>> directionalView;
    std::vector<Position> shellPositions;
    Position selfPosition{0, 0};
    Direction selfDirection = Direction(Direction::U);
    bool selfPositionSet = false;
};

#endif
