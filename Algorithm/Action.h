#ifndef ALGORITHM_318772340_206580102_ACTION_H
#define ALGORITHM_318772340_206580102_ACTION_H

#include "../common/ActionRequest.h"
#include "../UserCommon/Direction.h"

using namespace UserCommon_318772340_206580102;

namespace Algorithm_318772340_206580102 {

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

} // namespace Algorithm_318772340_206580102

#endif // ALGORITHM_318772340_206580102_ACTION_H 