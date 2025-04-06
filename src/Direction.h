#ifndef DIRECTION_H
#define DIRECTION_H

struct Position;  // ⬅️ Forward declaration במקום include

class Direction {
public:
    enum Value {
        U, UR, R, DR, D, DL, L, UL
    };

private:
    Value currentDirection;

public:
    Direction(Value dir) : currentDirection(dir) {}

    Position toVector() const;  // רק הכרזה, המימוש ילך ל־cpp

    Direction getOppositeDirection() const;

    void rotateClockwise8();
    void rotateClockwise4();
    void rotateCounterClockwise8();
    void rotateCounterClockwise4();

    Value getDirection() const;
    void setDirection(Value newDirection);
};

#endif // DIRECTION_H
