#ifndef ALGORITHM2_H
#define ALGORITHM2_H

#include "IAlgorithm.h"
#include "Board.h"
#include "Tank.h"
#include "Action.h"

class Algorithm2 : public IAlgorithm {
    public:
        Action nextAction(const Board& board, const Tank& self, const Tank& enemy) override;
};

#endif