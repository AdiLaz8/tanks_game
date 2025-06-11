#include "Position.h"
#include "Direction.h"
#include <iostream>

void Position::move(Direction dir, int width, int height) {
    Position delta = dir.toVector();
    x = (x + delta.x + width)%width ;
    y = (y + delta.y + height)%height;
}
void Position::setx(int xx){
    x=xx;
}
void Position::sety(int yy){
    y=yy;
}