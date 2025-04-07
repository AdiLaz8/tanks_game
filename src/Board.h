#ifndef BOARD_H
#define BOARD_H

#include "CellSlot.h"
#include "Shell.h"
#include <vector>

class Board {
private:
    int width, height;
    CellSlot** grid;
    std::vector<Shell*> shells;  // Maintaining a list of shells
    Tank* tank1;
    Tank* tank2;

public:
    Board(int w, int h);
    ~Board();

    CellSlot& getSlot(int x, int y) const;
    void addObject(Cell* obj, int x, int y);
    void removeObject(Cell* obj, int x, int y);
    bool isPassable(int x, int y) const;
    const std::vector<Shell*>& getShells() const { return shells; }
    int getWidth() const;
    int getHeight() const;
    Tank* getTank(int tankNumber);
};

#endif
