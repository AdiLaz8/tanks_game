#include "CellSlot.h"

void CellSlot::add(Cell* obj) {
    objects.push_back(obj);
}

void CellSlot::remove(Cell* obj) {
    objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
}

const std::vector<Cell*>& CellSlot::getAll() const {
    return objects;
}

Tank* CellSlot::getTank() const {
    for (Cell* obj : objects) {
        Tank* tank = dynamic_cast<Tank*>(obj);
        if (tank) return tank;
    }
    return nullptr;
}

Wall* CellSlot::getWall() const {
    for (Cell* obj : objects) {
        Wall* wall = dynamic_cast<Wall*>(obj);
        if (wall) return wall;
    }
    return nullptr;
}

Mine* CellSlot::getMine() const {
    for (Cell* obj : objects) {
        Mine* mine = dynamic_cast<Mine*>(obj);
        if (mine) return mine;
    }
    return nullptr;
}

std::vector<Shell*> CellSlot::getShells() const {
    std::vector<Shell*> shells;
    for (Cell* obj : objects) {
        Shell* shell = dynamic_cast<Shell*>(obj);
        if (shell) shells.push_back(shell);
    }
    return shells;
}

int CellSlot::countTanks() const {
    int count = 0;
    for (Cell* obj : objects) {
        if (dynamic_cast<Tank*>(obj)) count++;
    }
    return count;
}

int CellSlot::countShells() const {
    int count = 0;
    for (Cell* obj : objects) {
        if (dynamic_cast<Shell*>(obj)) count++;
    }
    return count;
}
