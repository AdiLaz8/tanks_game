#include "CellSlot.h"

// Adding any object to the cellslot
void CellSlot::addObject(std::unique_ptr<Cell> obj) {
    if (dynamic_cast<Shell*>(obj.get()))
        return; // לא שומרים shell כאן
    objects.push_back(std::move(obj));
}


// Removing any object from the cellslot
// void CellSlot::removeObject(Cell* obj) {
//     auto it = std::remove_if(objects.begin(), objects.end(),
//         [obj](const std::unique_ptr<Cell>& ptr) {
//             return ptr.get() == obj;
//         });
//     objects.erase(it, objects.end());
// }

void CellSlot::addShellPointerOnly(Shell* shell) {
    shellPointers.push_back(shell); // שדה חדש מסוג vector<Shell*>
}



void CellSlot::removeShellPointerOnly(Shell* shell) {
    shellPointers.erase(std::remove(shellPointers.begin(), shellPointers.end(), shell), shellPointers.end());
}






void CellSlot::removeTank() {
    auto it = std::remove_if(objects.begin(), objects.end(), [](const std::unique_ptr<Cell>& obj) {
        return dynamic_cast<Tank*>(obj.get()) != nullptr;
    });
    objects.erase(it, objects.end());
}

void CellSlot::removeWall() {
    auto it = std::remove_if(objects.begin(), objects.end(), [](const std::unique_ptr<Cell>& obj) {
        return dynamic_cast<Wall*>(obj.get()) != nullptr;
    });
    objects.erase(it, objects.end());
}

void CellSlot::removeMine() {
    auto it = std::remove_if(objects.begin(), objects.end(), [](const std::unique_ptr<Cell>& obj) {
        return dynamic_cast<Mine*>(obj.get()) != nullptr;
    });
    objects.erase(it, objects.end());
}


// Returns a vector of all the current objects inside the cellslot
const std::vector<std::unique_ptr<Cell>>& CellSlot::getAll() const {
    return objects;
}

// Returns the tank that's in the cell slot, or nulll pointer if there isn't one
Tank* CellSlot::getTank() const {
    for (const auto& obj : objects) {
        Tank* tank = dynamic_cast<Tank*>(obj.get());
        if (tank) return tank;
    }
    return nullptr;
}

// Returns the walll that's in the cell slot, or null pointer if there isn't one
Wall* CellSlot::getWall() const {
    for (const auto& obj : objects) {
        Wall* wall = dynamic_cast<Wall*>(obj.get());
        if (wall) return wall;
    }
    return nullptr;
}

// Returns the mine that's in the cell slot, or null pointer if there isn't one
Mine* CellSlot::getMine() const {
    for (const auto& obj : objects) {
        Mine* mine = dynamic_cast<Mine*>(obj.get());
        if (mine) return mine;
    }
    return nullptr;
}

// Returns a vector of all the shells that inside the cell slot in a given moment, will be useful to check collisions between two or more shells
std::vector<Shell*> CellSlot::getShells() const {
    std::vector<Shell*> shells;
    for (const auto& obj : objects) {
        Shell* shell = dynamic_cast<Shell*>(obj.get());
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
