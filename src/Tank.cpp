#include "Tank.h"

Tank::Tank(char owner, int ammunation, Direction d, Position p, int shooting_status, int backward_status, bool is_alive)
    : owner(owner), ammunation(ammunation), d(d), p(p), shooting_status(shooting_status), backward_status(backward_status), is_alive(is_alive) {}

char Tank::getSymbol() const {
    return owner;
}

const Position& Tank::getPosition() const {
    return p;
}

const Direction& Tank::getDirection() const {
    return d;
}

int Tank::getAmmo() const {
    return ammunation;
}

int Tank::getShootingStatus() const {
    return shooting_status;
}

int Tank::getBackwardStatus() const {
    return backward_status;
}

bool Tank::isAlive() const {
    return is_alive;
}

Position Tank::moveForward(int width, int height) {
    p.move(d, width, height);
    return p;
}

Position Tank::moveBackward(int width, int height) {
    p.move(d.getOppositeDirection(), width, height);
    return p;
}

void Tank::decreaseBackward() {
    backward_status--;
}

void Tank::setBackward(int status) {
    backward_status = status;
}

void Tank::shoot() {
    ammunation--;
    shooting_status = 4;
}

void Tank::hit() {
    is_alive = false;
}

void Tank::decreaseShooting() {
    shooting_status--;
}

void Tank::rotateLeft4() {
    d.rotateCounterClockwise4();
}

void Tank::rotateLeft8() {
    d.rotateCounterClockwise8();
}

void Tank::rotateRight4() {
    d.rotateClockwise4();
}

void Tank::rotateRight8() {
    d.rotateClockwise8();
}

void Tank::setPosition(Position newPos){
    p = newPos;
}