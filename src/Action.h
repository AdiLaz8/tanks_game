#ifndef ACTION_H
#define ACTION_H

#include "ActionRequest.h"
#include "Direction.h"

class Action {
private:
    ActionRequest type;
    Direction direction;  // Optional, depends on the action type

public:
    // Constructors
    Action(ActionRequest type) : type(type), direction(Direction::Value::U) {}  // Default direction
    Action(ActionRequest type, Direction direction) : type(type), direction(direction) {}

    // Accessors
    ActionRequest getType() const { return type; }
    Direction getDirection() const { return direction; }

    // Setters
    void setType(ActionRequest newType) { type = newType; }
    void setDirection(Direction newDirection) { direction = newDirection; }
};

#endif 