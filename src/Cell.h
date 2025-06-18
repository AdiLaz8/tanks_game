#ifndef CELL_H
#define CELL_H
#include <string>


class Cell {
public:
    virtual ~Cell() {}
    virtual std::string toString() const = 0;
    virtual char getSymbol() const = 0;  
};

#endif 