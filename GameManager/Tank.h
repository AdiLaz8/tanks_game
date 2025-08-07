#ifndef GAMEMANAGER_318772340_206580102_TANK_H
#define GAMEMANAGER_318772340_206580102_TANK_H

#include "Cell.h"
#include "../UserCommon/Direction.h"
#include "../UserCommon/Position.h"
#include <sstream>

using namespace UserCommon_318772340_206580102;

namespace GameManager_318772340_206580102 {

class Tank : public Cell {
private:
    char owner;
    Direction d;
    Position p;
    bool alive = true;
    int tankIndex = -1;
    int birthIndex = -1; 
    int backwardStatus = 0;
    int remainingShells = 0;
    void setDirection(Direction newDirection);
    
    
public:
    Tank(char owner, Direction d = Direction(Direction::L), Position p = Position(0, 0));
    virtual ~Tank();
    char getSymbol() const override;
    const Position& getPosition() const;
    const Direction& getDirection() const;
    Position moveForward(int width, int height);
    Position moveBackward(int width, int height);
    bool isAlive() const;
    void Hit();
    void rotateLeft4();
    void rotateRight4();
    void rotateLeft8();
    void rotateRight8();
    int getBackwardStatus() const { return backwardStatus; }
    void setBackwardStatus(int status) { backwardStatus = status; }
    void decreaseBackwardStatus() { if (backwardStatus > 0) backwardStatus--; }
    int getBirthIndex() const { return birthIndex; }
    void setBirthIndex(int index) { birthIndex = index; }
    int getTankIndex() const { return tankIndex; }
    void setTankIndex(int index) { tankIndex = index; }
    int getRemainingShells() const { return remainingShells; }
    void decreaseShells() { if (remainingShells > 0) --remainingShells; }
    void setRemainingShells(int n) { remainingShells = n; }
    void setPosition(const Position& newPos) { p = newPos; }
    std::string toString() const override;
    std::string getFullIdString() const;

};

} // namespace GameManager_318772340_206580102

#endif // GAMEMANAGER_318772340_206580102_TANK_H