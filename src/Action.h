#ifndef ACTION_H
#define ACTION_H

#include "ActionType.h"
#include "Direction.h"  // Include if actions need to specify a direction

class Action {
private:
    ActionType type;
    Direction direction;  // Optional, depends on the action type

public:
    // Constructors
    Action(ActionType type) : type(type), direction(Direction::Value::U) {}  // Default direction
    Action(ActionType type, Direction direction) : type(type), direction(direction) {}

    // Accessors
    ActionType getType() const { return type; }
    Direction getDirection() const { return direction; }

    // Setters
    void setType(ActionType newType) { type = newType; }
    void setDirection(Direction newDirection) { direction = newDirection; }
};

#endif // ACTION_H