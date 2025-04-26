#include "Wall.h"

Wall::Wall() : health(2) {}

// when a shell hits the wall, reduce it's health and return it's updated health
int Wall::onHit() {
    health--;
    return health;
}

char Wall::getSymbol() const {
    return '#';
}
