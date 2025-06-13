#include "CellSlot.h"
#include <iostream>

void CellSlot::addObject(std::unique_ptr<Cell> obj) {
    if (dynamic_cast<Shell*>(obj.get()))
        return; 
    objects.push_back(std::move(obj));
}

void CellSlot::addShellPointerOnly(Shell* shell) {
    shellPointers.push_back(shell); 
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

void CellSlot::removeTank(Tank* tank) {
    objects.erase(
        std::remove_if(objects.begin(), objects.end(),
            [tank](const std::unique_ptr<Cell>& obj) { return obj.get() == tank; }),
        objects.end()
    );
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

const std::vector<std::unique_ptr<Cell>>& CellSlot::getAll() const {
    return objects;
}

std::unique_ptr<Cell> CellSlot::extractTank(Tank* tank) {
    auto it = std::find_if(objects.begin(), objects.end(),
        [tank](const std::unique_ptr<Cell>& obj) { return obj.get() == tank; });
    if (it != objects.end()) {
        std::unique_ptr<Cell> ptr = std::move(*it);
        objects.erase(it);
        return ptr;
    }
    return nullptr;
}


Tank* CellSlot::getTank() const {
    for (const auto& obj : objects) {
        Tank* tank = dynamic_cast<Tank*>(obj.get());
        if (tank) return tank;
    }
    return nullptr;
}

Wall* CellSlot::getWall() const {
    for (const auto& obj : objects) {
        Wall* wall = dynamic_cast<Wall*>(obj.get());
        if (wall) return wall;
    }
    return nullptr;
}

Mine* CellSlot::getMine() const {
    for (const auto& obj : objects) {
        Mine* mine = dynamic_cast<Mine*>(obj.get());
        if (mine) return mine;
    }
    return nullptr;
}

std::vector<Shell*> CellSlot::getShells() const {
    return shellPointers;
}