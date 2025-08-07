#include "Position.h"
#include "Direction.h"
#include <iostream>

namespace UserCommon_318772340_206580102 {

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

} // namespace UserCommon_318772340_206580102