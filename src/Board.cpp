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
void Board::addObject(std::unique_ptr<Cell> obj, int x, int y) {
    Cell* rawPtr = obj.get();
    grid[y][x].addObject(std::move(obj));
    if (auto shell = dynamic_cast<Shell*>(rawPtr)) {
        shells.push_back(shell);
    }
    if (auto tank = dynamic_cast<Tank*>(rawPtr)) {
        if (tank->getSymbol() == '1') {
            tanks1.push_back(tank);
            }   
        else if (tank->getSymbol() == '2') {
            tanks2.push_back(tank);
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
    if (auto tank = dynamic_cast<Tank*>(obj)) {
        auto& vec = (tank->getSymbol() == '1') ? tanks1 : tanks2;
        auto it = std::find(vec.begin(), vec.end(), tank);
        if (it != vec.end()) {
            vec.erase(it);
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
const std::vector<Tank*>&::Board::getTanks(int playerId) const {
    return (playerId == 1) ? tanks1 : tanks2;
}