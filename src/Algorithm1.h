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
        Position lastEnemyPos = Position(-1, -1);
        Position lastComputePosition = Position(-1, -1);
    public:
        Action nextAction(const Board& board,const Tank& self, const Tank& enemy) override;
    private:
        std::vector<Direction::Value> computeBFS(const Board& board, const Tank& self, const Tank& enemy);
};

#endif