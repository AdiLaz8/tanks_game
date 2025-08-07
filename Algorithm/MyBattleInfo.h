#ifndef ALGORITHM_318772340_206580102_MY_BATTLE_INFO_H
#define ALGORITHM_318772340_206580102_MY_BATTLE_INFO_H

#include "../common/BattleInfo.h"
#include "../UserCommon/Position.h"
#include "../UserCommon/Direction.h"
#include <vector>

using namespace UserCommon_318772340_206580102;

namespace Algorithm_318772340_206580102 {

class MyBattleInfo : public BattleInfo {
public:
    MyBattleInfo(size_t rows, size_t cols, int numShells);
    void addObject(const Position& pos, char symbol, int playerId, int boardRows, int boardCols);
    const std::vector<std::pair<Position, char>>& getFullView() const;
    const std::vector<Position>& getShellPositions() const;
    Position getSelfPosition() const;
    void setSelfPosition(Position p);
    void setMinesPositions(const std::vector<Position>& positions) { minePositions = positions;}
    int getInitialShells() const;
    size_t getWidth() const;
    size_t getHeight() const;

private:
    size_t boardRows;
    size_t boardCols;
    int numShells;
    std::vector<std::pair<Position, char>> fullView;
    std::vector<Position> shellPositions;
    std::vector<Position> minePositions;
    Position selfPosition{0, 0};
    bool selfPositionSet = false;
    void reset();

};

} // namespace Algorithm_318772340_206580102

#endif // ALGORITHM_318772340_206580102_MY_BATTLE_INFO_H