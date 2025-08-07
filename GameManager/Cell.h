#ifndef GAMEMANAGER_318772340_206580102_CELL_H
#define GAMEMANAGER_318772340_206580102_CELL_H

#include <string>

namespace GameManager_318772340_206580102 {

class Cell {
public:
    virtual ~Cell() {}
    virtual std::string toString() const = 0;
    virtual char getSymbol() const = 0;  
};

} // namespace GameManager_318772340_206580102

#endif // GAMEMANAGER_318772340_206580102_CELL_H 