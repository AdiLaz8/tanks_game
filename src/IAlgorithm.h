#ifndef IALGORITHM_H
#define IALGORITHM_H

#include "Board.h"
#include "Action.h"
#include "Tank.h"

class IAlgorithm {
public:
    virtual Action nextAction(const Board& board, const Tank& self, const Tank& enemy) = 0;
    virtual ~IAlgorithm() = default;
};

#endif
