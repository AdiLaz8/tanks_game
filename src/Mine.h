#ifndef MINE_H
#define MINE_H

#include "Cell.h"
#include <string>

class Mine : public Cell
{
public:
    Mine() = default;
    virtual ~Mine() override {}
    char getSymbol() const override {
        return '@';
    }
    std::string toString() const override {
        return "Mine";
}

};
#endif