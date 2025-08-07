#ifndef USERCOMMON_318772340_206580102_DIRECTION_H
#define USERCOMMON_318772340_206580102_DIRECTION_H

#include <string>

namespace UserCommon_318772340_206580102 {

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
    std::string toString() const;
};

} // namespace UserCommon_318772340_206580102

#endif // USERCOMMON_318772340_206580102_DIRECTION_H
