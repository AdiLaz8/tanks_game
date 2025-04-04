#include "Wall.h"

Wall::Wall() : health(2) {}

int Wall::onHit() {
    health--;
    return health;
}

char Wall::getSymbol() const {
    return '#';
}
