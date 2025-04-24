#ifndef IALGORITHM_H
#define IALGORITHM_H

#include "Board.h"
#include "Action.h"
#include "Tank.h"

class IAlgorithm {
public:
    virtual Action nextAction(const Board& board, const Tank& self, const Tank& enemy) = 0;
    virtual ~IAlgorithm() = default;
    // Implemented method
    bool canShoot(const Tank& self, const Tank& enemy, const Board& board);
    ActionType rotateTowards(Direction::Value current, Direction::Value desired) const;
    Action moveIfThreatened(const Board& board, const Tank& self);
    bool isThreatenedByShells(const Board& board, const Position& pos);
    Direction::Value getDirectionTo(const Position& from, const Position& to) const;
};

#endif
