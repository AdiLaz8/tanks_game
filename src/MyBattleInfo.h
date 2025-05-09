#ifndef MY_BATTLE_INFO_H
#define MY_BATTLE_INFO_H

#include "BattleInfo.h"
#include "Position.h"
#include "Direction.h"
#include <vector>

class MyBattleInfo : public BattleInfo {
public:
    MyBattleInfo(size_t rows, size_t cols, int numShells);
    void reset();
    void addObject(const Position& pos, char symbol, int playerId, int boardRows, int boardCols);

    const std::vector<std::pair<Position, char>>& getFullView() const;
    const char (&getLocalView() const)[5][5];
    const std::vector<std::pair<Position, char>>& getDirectionalView() const;
    const std::vector<Position>& getShellPositions() const;
    Position getSelfPosition() const;
    Direction getSelfDirection() const;
    void setSelfDirection(Direction d);
    void setSelfPosition(Position p);
    size_t getWidth() { return boardCols; }
    size_t getHeight() { return boardRows; }

private:
    size_t boardRows;
    size_t boardCols;
    int numShells;
    std::vector<std::pair<Position, char>> fullView;
    char localView[5][5];
    std::vector<std::pair<Position, char>> directionalView;
    std::vector<Position> shellPositions;
    Position selfPosition{0, 0};
    Direction selfDirection = Direction(Direction::U);
    bool selfPositionSet = false;
};

#endif
