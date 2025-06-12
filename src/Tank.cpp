#include "Tank.h"

Tank::Tank(char owner, Direction d, Position p)
    : owner(owner), d(d), p(p) {}

Tank::~Tank() {}

char Tank::getSymbol() const {
    return owner;
}

const Position& Tank::getPosition() const {
    return p;
}

const Direction& Tank::getDirection() const {
    return d;
}

Position Tank::moveForward(int width, int height) {
    p.move(d, width, height);
    return p;
}

Position Tank::moveBackward(int width, int height) {
    p.move(d.getOppositeDirection(), width, height);
    return p;
}

void Tank::rotateLeft4() {
    d.rotateCounterClockwise4();
}

void Tank::rotateRight4() {
    d.rotateClockwise4();
}

void Tank::rotateLeft8() {
    d.rotateCounterClockwise8();
}

void Tank::rotateRight8() {
    d.rotateClockwise8();
}

// void Tank::setPosition(Position newPos) {
//     p = newPos;
// }

void Tank::setDirection(Direction newDirection) {
    d = newDirection;
}

std::string Tank::toString() const {
    std::stringstream ss;
    ss << "Tank(owner=" << owner
       << ", pos=(" << p.getx() << "," << p.gety() << ")"
       << ", dir=" << d.getDirection()
       << ")";
    return ss.str();
}
std::string Tank::getFullIdString() const {
    std::stringstream ss;
    ss << "Player " << owner << " Tank " << birthIndex;
    return ss.str();
}

bool Tank::isAlive() const { return alive; }

void Tank::Hit(){
    alive=false;
}