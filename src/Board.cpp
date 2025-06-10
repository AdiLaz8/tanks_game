#include "Board.h"
#include <typeinfo>
#include <iostream>
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
// Get the slot in some position, wrapping around if necessary
CellSlot& Board::getSlot(int x, int y) const{
    return grid[(y + height) % height][(x + width) % width];
}
// adding shell to the board
void Board::addShell(std::unique_ptr<Shell> shell) {
    Shell* raw = shell.get();
    shells.push_back(raw);
    ownedShells.push_back(std::move(shell)); 
    Position pos = raw->getPosition();
    grid[pos.gety()][pos.getx()].addShellPointerOnly(raw);

}

// moving shell to a new position
void Board::moveShellTo(Shell* shell, int oldX, int oldY, int newX, int newY) {
    grid[oldY][oldX].removeShellPointerOnly(shell);
    grid[newY][newX].addShellPointerOnly(shell);
}

// add an object to the board at a specific position
void Board::addObject(std::unique_ptr<Cell> obj, int x, int y) {
    Cell* raw = obj.get();
    if (auto* shell = dynamic_cast<Shell*>(raw)) {
        shells.push_back(shell);                
        ownedShells.push_back(std::unique_ptr<Shell>(shell)); 
        obj.release();                           
        grid[y][x].addShellPointerOnly(shell);  
        return;
    }
    grid[y][x].addObject(std::move(obj));

    if (auto* tank = dynamic_cast<Tank*>(raw)) {
        if (tank->getSymbol() == '1') tanks1.push_back(tank);
        else if (tank->getSymbol() == '2') tanks2.push_back(tank);
    }
}

// remove a shell pointer from the board at a specific position
void Board::removeShellPointerOnly(Shell* shell, const Position& pos) {
    grid[pos.gety()][pos.getx()].removeShellPointerOnly(shell);
}

// removing objects from the board
void Board::removeTankAt(int x, int y) {
    Tank* tank = grid[y][x].getTank();
    if (!tank) return;

    auto& vec = (tank->getSymbol() == '1') ? tanks1 : tanks2;
    vec.erase(std::remove(vec.begin(), vec.end(), tank), vec.end());

    grid[y][x].removeTank(); 
}

void Board::removeWallAt(int x, int y) {
    grid[y][x].removeWall();
}

void Board::removeMineAt(int x, int y) {
    grid[y][x].removeMine();
}

void Board::removeShell(Shell* shell, int x, int y) {
    grid[y][x].removeShellPointerOnly(shell);
    shells.erase(std::remove(shells.begin(), shells.end(), shell), shells.end());

    auto it = std::remove_if(ownedShells.begin(), ownedShells.end(),
        [shell](const std::unique_ptr<Shell>& ptr) { return ptr.get() == shell; });
    ownedShells.erase(it, ownedShells.end()); 
}

// check if the slot has wall (bad step) or if it's passable
bool Board::isPassable(int x, int y) const {
    const CellSlot& slot = grid[(y + height) % height][(x + width) % width];
    return !slot.getWall();
}

int Board::getWidth() const {
    return width;
}

int Board::getHeight() const {
    return height;
}

const std::vector<Tank*>&::Board::getTanks(int playerId) const {
    return (playerId == 1) ? tanks1 : tanks2;
}