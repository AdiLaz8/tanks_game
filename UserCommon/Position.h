#ifndef USERCOMMON_318772340_206580102_POSITION_H
#define USERCOMMON_318772340_206580102_POSITION_H

#include "Direction.h"

namespace UserCommon_318772340_206580102 {

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

} // namespace UserCommon_318772340_206580102

#endif // USERCOMMON_318772340_206580102_POSITION_H