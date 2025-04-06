#include "Board.h"

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
}

void Board::removeObject(Cell* obj, int x, int y) {
    grid[y][x].remove(obj);
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
