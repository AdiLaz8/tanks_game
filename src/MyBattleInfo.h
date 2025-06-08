#ifndef MY_BATTLE_INFO_H
#define MY_BATTLE_INFO_H

#include "BattleInfo.h"
#include "Position.h"
#include "Direction.h"
#include <vector>

class MyBattleInfo : public BattleInfo {
public:
    MyBattleInfo(size_t rows, size_t cols, int numShells);
    void addObject(const Position& pos, char symbol, int playerId, int boardRows, int boardCols);
    const std::vector<std::pair<Position, char>>& getFullView() const;
    const std::vector<Position>& getShellPositions() const;
    Position getSelfPosition() const;
    void setSelfPosition(Position p);
    int getInitialShells() const;
    size_t getWidth() const;
    size_t getHeight() const;
private:
    size_t boardRows;
    size_t boardCols;
    int numShells;
    std::vector<std::pair<Position, char>> fullView;
    std::vector<Position> shellPositions;
    Position selfPosition{0, 0};
    bool selfPositionSet = false;
    void reset();

};

#endif
