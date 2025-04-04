#ifndef TANK_H
#define TANK_H

#include "Cell.h"
#include "Direction.h"

class Tank: public Cell {
    private:
        char owner;
        int ammunation;
        Direction d;
        Position p;
        int shooting_status;
        int backward_status;
    public:
        Tank(char owner, int ammunation = 16, Direction d, Position p, int shooting_status = 0, int backward_status = -1) : owner(owner), ammunation(ammunation), d(d), p(p), shooting_status(shooting_status), backward_status(backward_status) {}
        char getSymbol() const override{
            return owner;
        }
        Position getPosition() const {
            return p;
        }
        Direction getDirection() const {
            return d;
        }
        int getAmmo() const {
            return ammunation;
        }
        int getShootingStatus() const {
            return shooting_status;
        }
        int getBackwardStatus() const {
            return backward_status;
        }
        Position moveForward() {
            if (backward_status >= 1){
                backward_status = -1;
                return p;
            }
            p.move(d);
            return p;
        }
        Position moveBackward() {
            p.move(d.getOppositeDirection());
            backward_status = 3;
        }
        void decreaseBackward() {
            backward_status = backward_status - 1;
        }
        void shoot() {
            ammunation = ammunation - 1;
            shooting_status = 4;
        }
        void decreaseShooting() {
            shooting_status = shooting_status - 1;
        }
        void rotateLeft4() {
            d.rotateCounterClockwise4();
        }
        void rotateLeft8() {
            d.rotateCounterClockwise8();
        }
        void rotateRight4() {
            d.rotateClockwise4();
        }
        void rotateRight8() {
            d.rotateClockwise8();
        }
};
#endif