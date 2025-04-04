#ifndef DIRECTION_H
#define DIRECTION_H

#include "Position.h"

class Direction {
public:
    enum Value {
        U,  // Up
        UR, // Up-Right
        R,  // Right
        DR, // Down-Right
        D,  // Down
        DL, // Down-Left
        L,  // Left
        UL  // Up-Left
    };

private:
    Value currentDirection;

public:
    Direction(Value dir) : currentDirection(dir) {}

    Position toVector() const {
        switch (currentDirection) {
            case U:  return Position(0, -1);
            case UR: return Position(1, -1);
            case R:  return Position(1, 0);
            case DR: return Position(1, 1);
            case D:  return Position(0, 1);
            case DL: return Position(-1, 1);
            case L:  return Position(-1, 0);
            case UL: return Position(-1, -1);
            default: return Position(0, 0); // Should never happen
        }
    }

    // Method to compute and return the opposite direction
    Direction getOppositeDirection() const {
        int oppositeIndex = (static_cast<int>(currentDirection) + 4) % 8;
        return Direction(static_cast<Value>(oppositeIndex));
    }

    void rotateClockwise8() {
        currentDirection = static_cast<Value>((static_cast<int>(currentDirection) + 1) % 8);
    }
    void rotateClockwise4() {
        currentDirection = static_cast<Value>((static_cast<int>(currentDirection) + 2) % 8);
    }

    void rotateCounterClockwise8() {
        currentDirection = static_cast<Value>((static_cast<int>(currentDirection) + 7) % 8);
    }
     void rotateCounterClockwise4() {
        currentDirection = static_cast<Value>((static_cast<int>(currentDirection) + 6) % 8);
    }

    Value getDirection() const {
        return currentDirection;
    }

    void setDirection(Value newDirection) {
        currentDirection = newDirection;
    }
};
#endif
