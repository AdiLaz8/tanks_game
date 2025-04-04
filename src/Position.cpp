#include "Position.h"
#include "Direction.h"

void Position::move(Direction dir,int width, int height) {
    Position delta = dir.toVector();
    x = (x + delta.x + width)%width ;
    y = (y + delta.y + height)%height;
}
