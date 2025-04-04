#ifndef CELL_H
#define CELL_H

#include "Position.h"

class Cell {
public:
    virtual ~Cell() {}

    virtual char getSymbol() const = 0;  // Return a character representation for each cell type
};

#endif // CELL_H
