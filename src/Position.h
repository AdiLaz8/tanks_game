#ifndef POSITION_H
#define POSITION_H
#include "Direction.h"

struct Position
{   
    private:
        int x;
        int y;
    public:
        Position(int x, int y) : x(x), y(y) {}
        bool operator==(const Position& other) const{
            return x==other.x && y==other.y;
        }

        Position operator+(const Position delta) const {
            return {x + delta.x, y + delta.y};
        }
        bool operator<(const Position& other) const {
        return (x < other.x) || (x == other.x && y < other.y);
    }
        void setx(int xx);
        void sety(int yy);
        void move(Direction dir,int width, int height);
        int getx() const {return x;}
        int gety() const{return y;}
    
};
#endif