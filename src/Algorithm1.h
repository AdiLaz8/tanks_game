#ifndef ALGO1_H
#define ALGO1_H

#include "IAlgorithm.h"
#include <vector>
#include "Direction.h"
#include "Position.h"
#include "Board.h"
#include "Tank.h"
class Algorithm1 : public IAlgorithm
{
    private:
        std::vector<Direction::Value> currentPath;
        Position lastEnemyPos = Position(0, 0);
    public:
        Action nextAction(const Board& board,const Tank& self, const Tank& enemy) override;
    private:
        std::vector<Direction::Value> computeBFS(const Board& board, const Tank& self, const Tank& enemy);
        Direction::Value getDirectionTo(const Position& from, const Position& to) const;
        ActionType rotateTowards(Direction::Value current, Direction::Value desired) const;
};

#endif