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
        std::vector<Direction::Value> currentPath; // the current path the BFS computed for us in order to get to the enemy tank
        Position lastEnemyPos = Position(-1, -1); // the last known position of the enemy tank
        Position lastComputePosition = Position(-1, -1);
        std::vector<Direction::Value> computeBFS(const Board& board, const Tank& self, const Tank& enemy);
    public:
        Action nextAction(const Board& board,const Tank& self, const Tank& enemy) override;
};

#endif