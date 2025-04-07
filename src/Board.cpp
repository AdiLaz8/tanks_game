#include "Board.h"
#include <typeinfo>

Board::Board(int w, int h) : width(w), height(h) {
    grid = new CellSlot*[height];
    for (int i = 0; i < height; ++i) {
        grid[i] = new CellSlot[width];
    }
}

Board::~Board() {
    for (int i = 0; i < height; ++i) {
        delete[] grid[i];
    }
    delete[] grid;
}

CellSlot& Board::getSlot(int x, int y) const{
    return grid[y][x];
}

void Board::addObject(Cell* obj, int x, int y) {
    grid[y][x].add(obj);
    // Check if the object is a shell and add it to the vector
    Shell* shell = dynamic_cast<Shell*>(obj);
    if (shell) {
        shells.push_back(shell);
    }
    if (auto tank = dynamic_cast<Tank*>(obj)) {
        if (tank->getSymbol() == '1') {
            tank1 = tank;
        } else if (tank->getSymbol() == '2') {
            tank2 = tank;
        }
    }
}

void Board::removeObject(Cell* obj, int x, int y) {
    grid[y][x].remove(obj);
    // Check if the object is a shell and remove it from the vector
    Shell* shell = dynamic_cast<Shell*>(obj);
    if (shell) {
        auto it = std::find(shells.begin(), shells.end(), shell);
        if (it != shells.end()) {
            shells.erase(it);
        }
    }
}

bool Board::isPassable(int x, int y) const {
    const CellSlot& slot = grid[y][x];
    return !slot.getWall() && !slot.getTank();
}

int Board::getWidth() const {
    return width;
}

int Board::getHeight() const {
    return height;
}

Tank* Board::getTank(int tankNumber) {
    if (tankNumber == 1) {
        return tank1;
    } else if (tankNumber == 2) {
        return tank2;
    }
    return nullptr; // Return null if no valid tank number
}
