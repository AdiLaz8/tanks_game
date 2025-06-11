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
    static int globalIdCounter;
    char ownerId; 
    int id;
    int getId() const { return id; }
    char getSymbol() const override;

public:
    Shell(Position p, Direction d, char ownerId);
    ~Shell();
    const Position& getPosition() const;
    void move(int width, int height);
    Direction getDirection() const;
    char getOwnerId() const;
    std::string toString() const override;
    void setPosition(const Position& newPos) { p = newPos; }
};

#endif