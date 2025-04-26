#include "Position.h"
#include "Direction.h"

// moves the position in the direction given, considering the width and height of the board to control tunnels
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