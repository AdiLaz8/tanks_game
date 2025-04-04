#ifndef CELLSLOT_H
#define CELLSLOT_H

#include <vector>
#include <algorithm>
#include "Cell.h"
#include "Tank.h"
#include "Wall.h"
#include "Mine.h"
#include "Shell.h"

class CellSlot{
    private:
        std::vector<Cell*> objects;
    public:
        void add(Cell* obj){
            objects.push_back(obj);
        }
        void remove(Cell* obj){
            auto temp = std::remove(objects.begin(), objects.end(), obj);
            objects.erase(temp, objects.end());
        }
        const std::vector<Cell*>& getAll() const {
            return objects;
        }
        Tank* getTank() const {
            for (Cell* obj : objects) {
                Tank* t = dynamic_cast<Tank*>(obj);
                if (t) return t;
            }
            return nullptr;
        }
        Wall* getWall() const {
            for (Cell* obj : objects) {
                Wall* w = dynamic_cast<Wall*>(obj);
                if (w) return w;
            }
            return nullptr;
        }
        Mine* getMine() const {
            for (Cell* obj : objects) {
                Mine* m = dynamic_cast<Mine*>(obj);
                if (m) return m;
            }
            return nullptr;
        }
        Shell* getShell() const {
            for (Cell* obj : objects) {
                Shell* s = dynamic_cast<Shell*>(obj);
                if (s) return s;
            }
            return nullptr;
        }
};
#endif
