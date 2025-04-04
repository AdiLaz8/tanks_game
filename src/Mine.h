#ifndef MINE_H
#define MINE_H

#include "Cell.h"
class Mine : public Cell
{
public:
    Mine() = default;
    char getSymbol() const override {
        return '@';
    }
};
#endif

