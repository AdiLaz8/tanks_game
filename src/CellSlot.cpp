#include "CellSlot.h"

void CellSlot::add(Cell* obj) {
    objects.push_back(obj);
}

void CellSlot::remove(Cell* obj) {
    auto temp = std::remove(objects.begin(), objects.end(), obj);
    objects.erase(temp, objects.end());
}

const std::vector<Cell*>& CellSlot::getAll() const {
    return objects;
}

Tank* CellSlot::getTank() const {
    for (Cell* obj : objects) {
        if (auto* t = dynamic_cast<Tank*>(obj)) return t;
    }
    return nullptr;
}

Wall* CellSlot::getWall() const {
    for (Cell* obj : objects) {
        if (auto* w = dynamic_cast<Wall*>(obj)) return w;
    }
    return nullptr;
}

Mine* CellSlot::getMine() const {
    for (Cell* obj : objects) {
        if (auto* m = dynamic_cast<Mine*>(obj)) return m;
    }
    return nullptr;
}

Shell* CellSlot::getShell() const {
    for (Cell* obj : objects) {
        if (auto* s = dynamic_cast<Shell*>(obj)) return s;
    }
    return nullptr;
}
