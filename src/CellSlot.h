#ifndef CELLSLOT_H
#define CELLSLOT_H

#include <vector>
#include <algorithm>
#include "Cell.h"
#include "Tank.h"
#include "Wall.h"
#include "Mine.h"
#include "Shell.h"
#include <memory>

class CellSlot {
private:
    std::vector<std::unique_ptr<Cell>> objects;
    std::vector<Shell*> shellPointers;

public:
    void addObject(std::unique_ptr<Cell> obj);
    void removeTank();
    void removeTank(Tank* tank); // for tank-tank collision
    void removeWall();
    void removeMine();
    void removeShell(Shell* shell);
    const std::vector<std::unique_ptr<Cell>>& getAll() const;
    void removeShellPointerOnly(Shell* shell);
    void addShellPointerOnly(Shell* shell);
    Tank* getTank() const;
    Wall* getWall() const;
    Mine* getMine() const;
    std::vector<Shell*> getShells() const;
};

#endif