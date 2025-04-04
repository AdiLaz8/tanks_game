#include "Position.h"
#include "Direction.h"

void Position::move(Direction dir) {
    Position delta = dir.toVector();
    x += delta.x;
    y += delta.y;
}
