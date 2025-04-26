#ifndef CELLSLOT_H
#define CELLSLOT_H

#include <vector>
#include <algorithm>
#include "Cell.h"
#include "Tank.h"
#include "Wall.h"
#include "Mine.h"
#include "Shell.h"

class CellSlot {
private:
    std::vector<Cell*> objects;

public:
    void addObject(Cell* obj);
    void removeObject(Cell* obj);
    const std::vector<Cell*>& getAll() const;

    Tank* getTank() const;
    Wall* getWall() const;
    Mine* getMine() const;
    std::vector<Shell*> getShells() const;
    // for now the next functions aren't useful, but we keep this in comment in case we'll need them in the next assignment
    // int countTanks() const;
    // int countShells() const;
};

#endif