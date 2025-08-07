#ifndef GAMEMANAGER_318772340_206580102_WALL_H
#define GAMEMANAGER_318772340_206580102_WALL_H

#include "Cell.h"
#include <sstream>

namespace GameManager_318772340_206580102 {

class Wall : public Cell {
private:
    int health;
public:
    Wall();
    virtual ~Wall() override {}
    int onHit();
    char getSymbol() const override;
    std::string toString() const override {
        std::stringstream ss;
        ss << "Wall(health=" << health << ")";
        return ss.str();
}

};

} // namespace GameManager_318772340_206580102

#endif // GAMEMANAGER_318772340_206580102_WALL_H