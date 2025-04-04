#include "Position.h"
#include "Direction.h"

Position Position::move(Direction dir) const {
    Position delta = dir.toVector();
    return *this + delta;
}
