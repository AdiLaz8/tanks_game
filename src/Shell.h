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
    int ownerId;

public:
    Shell(Position p, Direction d, int ownerId);

    char getSymbol() const override;

    const Position& getPosition() const;

    void move(int width, int height);

    Direction getDirection() const;
    int getOwnerId() const;
    std::string toString() const override {
        std::stringstream ss;
        ss << "Shell(owner=" << ownerId
            << ", dir=" << d.getDirection()
            << ", pos=(" << p.x << "," << p.y << "))";
        return ss.str();
}

};

#endif
