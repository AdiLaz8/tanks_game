#include "Board.h"
#include <typeinfo>

Board::Board(int w, int h) : width(w), height(h) {
    grid = new CellSlot*[height];
    for (int i = 0; i < height; ++i) {
        grid[i] = new CellSlot[width];
    }
}

// constructs a new board with empty grids
Board::~Board() {
    for (int i = 0; i < height; ++i) {
        delete[] grid[i];
    }
    delete[] grid;
}

// returns the cellslot in this position
CellSlot& Board::getSlot(int x, int y) const{
    return grid[(y + height) % height][(x + width) % width];
}

// add any object to the grid
void Board::addObject(Cell* obj, int x, int y) {
    grid[y][x].addObject(obj);
    // Check if the object is a shell and add it to the vector of shells
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

// remove any object from the grid
void Board::removeObject(Cell* obj, int x, int y) {
    grid[y][x].removeObject(obj);
    // Check if the object is a shell and remove it from the vector of shells
    Shell* shell = dynamic_cast<Shell*>(obj);
    if (shell) {
        auto it = std::find(shells.begin(), shells.end(), shell);
        if (it != shells.end()) {
            shells.erase(it);
        }
    }
}

// checks and returns if the cellslot in this position has a mine or a wall
bool Board::isPassable(int x, int y) const {
    const CellSlot& slot = grid[(y + height) % height][(x + width) % width];
    return !slot.getWall() && !slot.getTank();
}

int Board::getWidth() const {
    return width;
}

int Board::getHeight() const {
    return height;
}

// returns the tank of the player required
Tank* Board::getTank(int tankNumber) {
    if (tankNumber == 1) {
        return tank1;
    } else if (tankNumber == 2) {
        return tank2;
    }
    return nullptr; // Return null if no valid tank number
}
