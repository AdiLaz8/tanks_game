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
    void add(Cell* obj);
    void remove(Cell* obj);
    const std::vector<Cell*>& getAll() const;

    Tank* getTank() const;
    Wall* getWall() const;
    Mine* getMine() const;
    Shell* getShell() const;
};

#endif
