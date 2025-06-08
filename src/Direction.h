#ifndef DIRECTION_H
#define DIRECTION_H

struct Position;  

class Direction {
public:
    enum Value {
        U, UR, R, DR, D, DL, L, UL
    };

private:
    Value currentDirection;

public:
    bool operator==(const Direction& other) const{
        return currentDirection==other.currentDirection;
    }
    Direction(Value dir) : currentDirection(dir) {}
    Position toVector() const; 
    Direction getOppositeDirection() const;
    void rotateClockwise8();
    void rotateClockwise4();
    void rotateCounterClockwise8();
    void rotateCounterClockwise4();
    Value getDirection() const;
    void setDirection(Value newDirection);
};

#endif
