#include "Wall.h"

namespace GameManager_318772340_206580102 {
Wall::Wall() : health(2) {}

int Wall::onHit() {
    health--;
    return health;
}

char Wall::getSymbol() const {
    return '#';
}

} // namespace GameManager_318772340_206580102