// #ifndef TANK_H
// #define TANK_H

// #include "Cell.h"
// #include "Direction.h"
// #include "Position.h"
// #include <sstream>

// class Tank : public Cell {
// private:
//     char owner; // 1 if it's a tank of player 1 and 2 if it's a tank of player 2
//     int ammunation; // how much shells the tanks has in its inventory
//     Direction d; // direction of the cannon of the tank - where he shoots and moves
//     Position p; // the current position of the tank on the board
//     int shooting_status; // a number that tells us if the tank can shoot if it's 0 or if it's in cooldown
//     int backward_status; // a number that tells us if the tank can move backwards if it's 0 or if it's in cooldown
//     bool is_alive; // weather the tank alive and in the game or it's dead

// public:
//     Tank(char owner, int ammunation = 16, Direction d = Direction(Direction::L), Position p = Position(0, 0), int shooting_status = 0, int backward_status = 0, bool is_alive = true);
//     virtual ~Tank();

//     char getSymbol() const override;
//     const Position& getPosition() const;
//     const Direction& getDirection() const;
//     int getAmmo() const;
//     int getShootingStatus() const;
//     int getBackwardStatus() const;
//     bool isAlive() const;

//     Position moveForward(int width, int height);
//     Position moveBackward(int width, int height);

//     void decreaseBackward();
//     void setBackward(int status);
//     void shoot();
//     void decreaseShooting();
//     void hit();
//     void setPosition(Position newPos);
//     void setDirection(Direction newDirection);

//     void rotateLeft4();
//     void rotateLeft8();
//     void rotateRight4();
//     void rotateRight8();
//     std::string toString() const override {
//         std::stringstream ss;
//         ss << "Tank(owner=" << owner
//         << ", ammo=" << ammunation
//         << ", pos=(" << p.getx() << "," << p.gety() << ")"
//         << ", dir=" << d.getDirection()
//         << ", shoot=" << shooting_status
//         << ", back=" << backward_status
//         << ")";
//         return ss.str();
// }

// };

// #endif

#ifndef TANK_H
#define TANK_H

#include "Cell.h"
#include "Direction.h"
#include "Position.h"
#include <sstream>

class Tank : public Cell {
private:
    char owner; // '1' or '2'
    Direction d;
    Position p;

public:
    Tank(char owner, Direction d = Direction(Direction::L), Position p = Position(0, 0));
    virtual ~Tank();

    char getSymbol() const override;
    const Position& getPosition() const;
    const Direction& getDirection() const;

    Position moveForward(int width, int height);
    Position moveBackward(int width, int height);

    void rotateLeft4();
    void rotateRight4();
    void rotateLeft8();
    void rotateRight8();

    void setPosition(Position newPos);
    void setDirection(Direction newDirection);

    std::string toString() const override;
};

#endif
