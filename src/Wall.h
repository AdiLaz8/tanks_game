#ifndef WALL_H
#define WALL_H

#include "Cell.h"
class Wall: public Cell{
    private:
        int health;
    public:
        Wall() : health(2) {};
        int onHit() {
            health--;
            return health;
        }
        char getSymbol() const override {
            return '#';
        }

};
#endif