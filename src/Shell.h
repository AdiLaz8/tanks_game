#ifndef SHELL_H
#define SHELL_H

#include "Cell.h"
#include "Position.h"
#include "Direction.h"

class Shell : public Cell {
private:
    Position p;
    Direction d;
    int ownerId;

public:
    Shell(Position p, Direction d, int ownerId);

    char getSymbol() const override;

    const Position& getPosition() const;

    void move(int width, int height);

    Direction getDirection() const;
    int getOwnerId() const;
};

#endif
