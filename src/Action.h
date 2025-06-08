#ifndef ACTION_H
#define ACTION_H

#include "ActionRequest.h"
#include "Direction.h"

class Action {
private:
    ActionRequest type;
    Direction direction; 

public:
    Action(ActionRequest type) : type(type), direction(Direction::Value::U) {} 
    Action(ActionRequest type, Direction direction) : type(type), direction(direction) {}

    ActionRequest getType() const { return type; }
    Direction getDirection() const { return direction; }

    void setType(ActionRequest newType) { type = newType; }
    void setDirection(Direction newDirection) { direction = newDirection; }
};

#endif 