#include "Shell.h"

int Shell::globalIdCounter = 1;
Shell::Shell(Position p, Direction d, char ownerId)
    : p(p), d(d), ownerId(ownerId), id(globalIdCounter++) {}

char Shell::getSymbol() const {
    return '*';
}
const Position& Shell::getPosition() const {
    return p;
}
Shell::~Shell() {
}
void Shell::move(int width, int height) {
    p.move(d, width, height);
}

std::string Shell::toString() const {
    std::stringstream ss;
    ss << "Shell(owner=" << ownerId
       << ", dir=" << d.getDirection()
       << ", pos=(" << p.getx() << "," << p.gety() << "))";
    return ss.str();
}

Direction Shell::getDirection() const {
    return d;
}

char Shell::getOwnerId() const {
    return ownerId;
}
