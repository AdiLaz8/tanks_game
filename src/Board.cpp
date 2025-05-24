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

// Board.cpp

void Board::addShell(std::unique_ptr<Shell> shell) {
    Shell* raw = shell.get();
    shells.push_back(raw);
    ownedShells.push_back(std::move(shell)); // נשמרת בעלות אמיתית
    Position pos = raw->getPosition();
    grid[pos.gety()][pos.getx()].addShellPointerOnly(raw);
}


void Board::moveShellTo(Shell* shell, int oldX, int oldY, int newX, int newY) {
    grid[oldY][oldX].removeShellPointerOnly(shell);
    grid[newY][newX].addShellPointerOnly(shell);
}


void Board::addObject(std::unique_ptr<Cell> obj, int x, int y) {
    Cell* raw = obj.get();
    
    if (auto* shell = dynamic_cast<Shell*>(raw)) {
        shells.push_back(shell);                 // שמירה בגישה מהירה
        ownedShells.push_back(std::unique_ptr<Shell>(shell)); // בעלות
        obj.release();                           // מניעת double delete
        grid[y][x].addShellPointerOnly(shell);   // נרשום רק את המצביע
        return;
    }

    // Wall / Tank / Mine:
    grid[y][x].addObject(std::move(obj));

    if (auto* tank = dynamic_cast<Tank*>(raw)) {
        if (tank->getSymbol() == '1') tanks1.push_back(tank);
        else if (tank->getSymbol() == '2') tanks2.push_back(tank);
    }
}

void Board::removeShellPointerOnly(Shell* shell, const Position& pos) {
    grid[pos.gety()][pos.getx()].removeShellPointerOnly(shell);
}

void Board::removeTankAt(int x, int y) {
    Tank* tank = grid[y][x].getTank();
    if (!tank) return;

    auto& vec = (tank->getSymbol() == '1') ? tanks1 : tanks2;
    vec.erase(std::remove(vec.begin(), vec.end(), tank), vec.end());

    grid[y][x].removeTank(); // ימחק גם את unique_ptr
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
    ownedShells.erase(it, ownedShells.end()); // זה משמיד את הפגז בזיכרון
}






// // remove any object from the grid
// void Board::removeObject(Cell* obj, int x, int y) {
//     grid[y][x].removeObject(obj);
//     // Check if the object is a shell and remove it from the vector of shells
//     Shell* shell = dynamic_cast<Shell*>(obj);
//     if (shell) {
//         auto it = std::find(shells.begin(), shells.end(), shell);
//         if (it != shells.end()) {
//             shells.erase(it);
//         }
//     }
//     if (auto tank = dynamic_cast<Tank*>(obj)) {
//         auto& vec = (tank->getSymbol() == '1') ? tanks1 : tanks2;
//         auto it = std::find(vec.begin(), vec.end(), tank);
//         if (it != vec.end()) {
//             vec.erase(it);
//         }
//     }
// }

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