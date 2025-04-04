#ifndef SHELL_H
#define SHELL_H

#include "Cell.h"
#include "Position.h"
#include "Direction.h"
class Shell : public Cell
{
    private:
        Position p;
        Direction d;
        int ownerId;
    public:
        Shell(Position p, Direction d, int ownerId): p(p), d(d), ownerId(ownerId) {}

        char getSymbol() const override {
            return '*';
        }

        Position getPosition() const {
            return p;
        }

        void move() {
            p.move(d);
        }

        Direction getDirection() const {
            return d;
        }

        int getOwnerId() const {
            return ownerId;
        }

};
#endif