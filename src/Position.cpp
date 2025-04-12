#include "Position.h"
#include "Direction.h"

void Position::move(Direction dir, int width, int height) {
    Position delta = dir.toVector();
    if (dir == Direction(Direction::DR) && x ==  - 1){
        x = 0;
        y = 0;
    }
    else{
        x = (x + delta.x + width)%width ;
        y = (y + delta.y + height)%height;
    }
}