#ifndef GAMEMANAGER_318772340_206580102_CELLSLOT_H
#define GAMEMANAGER_318772340_206580102_CELLSLOT_H

#include <vector>
#include <algorithm>
#include "Cell.h"
#include "Tank.h"
#include "Wall.h"
#include "Mine.h"
#include "Shell.h"
#include <memory>

namespace GameManager_318772340_206580102 {

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
    std::unique_ptr<Cell> extractTank(Tank* tank);
    Tank* getTank() const;
    Wall* getWall() const;
    Mine* getMine() const;
    std::vector<Shell*> getShells() const;
};

} // namespace GameManager_318772340_206580102

#endif // GAMEMANAGER_318772340_206580102_CELLSLOT_H