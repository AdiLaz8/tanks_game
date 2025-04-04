#ifndef BOARD_H
#define BOARD_H

#include "CellSlot.h"

class Board {
private:
    int width, height;
    CellSlot** grid;

public:
    Board(int w, int h);
    ~Board();

    CellSlot& getSlot(int x, int y);
    void addObject(Cell* obj, int x, int y);
    void removeObject(Cell* obj, int x, int y);
    bool isPassable(int x, int y) const;

    int getWidth() const;
    int getHeight() const;
};

#endif
