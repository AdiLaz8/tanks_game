#ifndef WALL_H
#define WALL_H

#include "Cell.h"
#include <sstream>

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

#endif