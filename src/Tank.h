#ifndef TANK_H
#define TANK_H
#include "Cell.h"
#include "Direction.h"
#include "Position.h"
#include <sstream>
class Tank : public Cell {
private:
    char owner;
    Direction d;
    Position p;
    bool alive = true;
    void setPosition(Position newPos);
    void setDirection(Direction newDirection);
public:
    Tank(char owner, Direction d = Direction(Direction::L), Position p = Position(0, 0));
    virtual ~Tank();
    int birthIndex = -1;
    char getSymbol() const override;
    const Position& getPosition() const;
    const Direction& getDirection() const;
    Position moveForward(int width, int height);
    Position moveBackward(int width, int height);
    bool isAlive() const;
    void Hit();
    void rotateLeft4();
    void rotateRight4();
    void rotateLeft8();
    void rotateRight8();
    std::string toString() const override;
};
#endif
