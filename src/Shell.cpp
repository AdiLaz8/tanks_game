#include "Shell.h"

Shell::Shell(Position p, Direction d, char ownerId)
    : p(p), d(d), ownerId(ownerId) {}

char Shell::getSymbol() const {
    return '*';
}

const Position& Shell::getPosition() const {
    return p;
}



void Shell::move(int width, int height) {
    p.move(d, width, height);
}

Direction Shell::getDirection() const {
    return d;
}

char Shell::getOwnerId() const {
    return ownerId;
}
