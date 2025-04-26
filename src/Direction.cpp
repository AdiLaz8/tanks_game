#include "Direction.h"
#include "Position.h"

// Returns a vector movemenet based on the direction we want to go
Position Direction::toVector() const {
    switch (currentDirection) {
        case U:  return Position(0, -1);
        case UR: return Position(1, -1);
        case R:  return Position(1, 0);
        case DR: return Position(1, 1);
        case D:  return Position(0, 1);
        case DL: return Position(-1, 1);
        case L:  return Position(-1, 0);
        case UL: return Position(-1, -1);
        default: return Position(0, 0); // not supposed to happen
    }
}

// Rotate functions
void Direction::rotateClockwise8() {
    currentDirection = static_cast<Value>((static_cast<int>(currentDirection) + 1) % 8);
}

void Direction::rotateClockwise4() {
    currentDirection = static_cast<Value>((static_cast<int>(currentDirection) + 2) % 8);
}

void Direction::rotateCounterClockwise8() {
    currentDirection = static_cast<Value>((static_cast<int>(currentDirection) + 7) % 8);
}

void Direction::rotateCounterClockwise4() {
    currentDirection = static_cast<Value>((static_cast<int>(currentDirection) + 6) % 8);
}

// Returns the opposite direction to the direction we currently face
Direction Direction::getOppositeDirection() const {
    int oppositeIndex = (static_cast<int>(currentDirection) + 4) % 8;
    return Direction(static_cast<Value>(oppositeIndex));
}

Direction::Value Direction::getDirection() const {
    return currentDirection;
}

void Direction::setDirection(Value newDirection) {
    currentDirection = newDirection;
}