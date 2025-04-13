#ifndef ALGORITHM2_H
#define ALGORITHM2_H

#include "IAlgorithm.h"
#include "Board.h"
#include "Tank.h"
#include "Action.h"

class Algorithm2 : public IAlgorithm {
    public:
        Direction::Value getDirectionTo(const Position& from, const Position& to) const ;
        Action nextAction(const Board& board, const Tank& self, const Tank& enemy) override;
        ActionType rotateTowards(const Direction::Value current, Direction::Value desired) const;

    private:
        Action moveIfThreatened(const Board& board, const Tank& self);
        bool isThreatenedByShells(const Board& board, const Position& pos);
};

#endif // ALGORITHM2_H
