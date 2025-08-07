#ifndef GAMEMANAGER_318772340_206580102_SHELL_H
#define GAMEMANAGER_318772340_206580102_SHELL_H

#include "Cell.h"
#include "../UserCommon/Position.h"
#include "../UserCommon/Direction.h"
#include <sstream>

using namespace UserCommon_318772340_206580102;

namespace GameManager_318772340_206580102 {

class Shell : public Cell {
private:
    Position p;
    Direction d;
    static int globalIdCounter;
    char ownerId; 
    int id;
    int getId() const { return id; }
    char getSymbol() const override;

public:
    Shell(Position p, Direction d, char ownerId);
    ~Shell();
    const Position& getPosition() const;
    void move(int width, int height);
    Direction getDirection() const;
    char getOwnerId() const;
    std::string toString() const override;
    void setPosition(const Position& newPos) { p = newPos; }
};

} // namespace GameManager_318772340_206580102

#endif // GAMEMANAGER_318772340_206580102_SHELL_H