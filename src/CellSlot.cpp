#include "CellSlot.h"

// Adding any object to the cellslot
void CellSlot::addObject(Cell* obj) {
    objects.push_back(obj);
}

// Removing any object from the cellslot
void CellSlot::removeObject(Cell* obj) {
    objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
}

// Returns a vector of all the current objects inside the cellslot
const std::vector<Cell*>& CellSlot::getAll() const {
    return objects;
}

// Returns the tank that's in the cell slot, or nulll pointer if there isn't one
Tank* CellSlot::getTank() const {
    for (Cell* obj : objects) {
        Tank* tank = dynamic_cast<Tank*>(obj);
        if (tank) return tank;
    }
    return nullptr;
}

// Returns the walll that's in the cell slot, or null pointer if there isn't one
Wall* CellSlot::getWall() const {
    for (Cell* obj : objects) {
        Wall* wall = dynamic_cast<Wall*>(obj);
        if (wall) return wall;
    }
    return nullptr;
}

// Returns the mine that's in the cell slot, or null pointer if there isn't one
Mine* CellSlot::getMine() const {
    for (Cell* obj : objects) {
        Mine* mine = dynamic_cast<Mine*>(obj);
        if (mine) return mine;
    }
    return nullptr;
}

// Returns a vector of all the shells that inside the cell slot in a given moment, will be useful to check collisions between two or more shells
std::vector<Shell*> CellSlot::getShells() const {
    std::vector<Shell*> shells;
    for (Cell* obj : objects) {
        Shell* shell = dynamic_cast<Shell*>(obj);
        if (shell) shells.push_back(shell);
    }
    return shells;
}

// for now the next functions aren't useful, but we keep this in comment in case we'll need them in the next assignment
// int CellSlot::countTanks() const {
//     int count = 0;
//     for (Cell* obj : objects) {
//         if (dynamic_cast<Tank*>(obj)) count++;
//     }
//     return count;
// }

// int CellSlot::countShells() const {
//     int count = 0;
//     for (Cell* obj : objects) {
//         if (dynamic_cast<Shell*>(obj)) count++;
//     }
//     return count;
// }
