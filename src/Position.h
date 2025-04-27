#ifndef POSITION_H
#define POSITION_H
#include "Direction.h"

struct Position
{
    int x;
    int y;
    Position(int x, int y) : x(x), y(y) {}
    bool operator==(const Position& other) const{
        return x==other.x && y==other.y;
    }

    Position operator+(const Position delta) const {
        return {x + delta.x, y + delta.y};
    }
    
    void move(Direction dir,int width, int height);
    
};
#endif