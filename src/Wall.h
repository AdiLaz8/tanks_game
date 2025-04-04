#ifndef WALL_H
#define WALL_H

#include "Cell.h"

class Wall : public Cell {
private:
    int health;

public:
    Wall();
    int onHit();
    char getSymbol() const override;
};

#endif
