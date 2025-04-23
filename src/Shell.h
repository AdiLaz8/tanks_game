#ifndef SHELL_H
#define SHELL_H

#include "Cell.h"
#include "Position.h"
#include "Direction.h"
#include <sstream>


class Shell : public Cell {
private:
    Position p;
    Direction d;
    char ownerId;

public:
    Shell(Position p, Direction d, char ownerId);
    ~Shell();


    char getSymbol() const override;

    const Position& getPosition() const;

    void move(int width, int height);

    Direction getDirection() const;
    char getOwnerId() const;
    std::string toString() const override;


};

#endif
