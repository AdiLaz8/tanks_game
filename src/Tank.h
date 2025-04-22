#ifndef TANK_H
#define TANK_H

#include "Cell.h"
#include "Direction.h"
#include "Position.h"
#include <sstream>

class Tank : public Cell {
private:
    char owner;
    int ammunation;
    Direction d;
    Position p;
    int shooting_status;
    int backward_status;
    bool is_alive;

public:
    Tank(char owner, int ammunation = 16, Direction d = Direction(Direction::L), Position p = Position(0, 0), int shooting_status = 0, int backward_status = 0, bool is_alive = true);

    char getSymbol() const override;

    const Position& getPosition() const;
    const Direction& getDirection() const;
    int getAmmo() const;
    int getShootingStatus() const;
    int getBackwardStatus() const;
    bool isAlive() const;

    Position moveForward(int width, int height);
    Position moveBackward(int width, int height);

    void decreaseBackward();
    void setBackward(int status);
    void shoot();
    void decreaseShooting();
    void hit();
    void setPosition(Position newPos);
    void setDirection(Direction newDirection);

    void rotateLeft4();
    void rotateLeft8();
    void rotateRight4();
    void rotateRight8();
    std::string toString() const override {
        std::stringstream ss;
        ss << "Tank(owner=" << owner
        << ", ammo=" << ammunation
        << ", pos=(" << p.x << "," << p.y << ")"
        << ", dir=" << d.getDirection()
        << ", shoot=" << shooting_status
        << ", back=" << backward_status
        << ")";
        return ss.str();
}

};

#endif
