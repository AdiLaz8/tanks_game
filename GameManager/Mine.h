#ifndef GAMEMANAGER_318772340_206580102_MINE_H
#define GAMEMANAGER_318772340_206580102_MINE_H

#include "Cell.h"
#include <string>

namespace GameManager_318772340_206580102 {

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

} // namespace GameManager_318772340_206580102

#endif // GAMEMANAGER_318772340_206580102_MINE_H