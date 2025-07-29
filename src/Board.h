#ifndef BOARD_H
#define BOARD_H

#include "CellSlot.h"
#include "Shell.h"
#include <vector>
#include <memory>


class Board {
private:
    int width, height;
    std::vector<std::vector<CellSlot>> grid;
    std::vector<Shell*> shells;  
    std::vector<Tank*> tanks1;
    std::vector<Tank*> tanks2;
    std::vector<std::unique_ptr<Shell>> ownedShells;

public:
    Board(int w, int h);
    Board(const Board&) = delete;
    Board& operator=(const Board&) = delete;
    Board(Board&&) = delete;
    Board& operator=(Board&&) = delete;
    CellSlot& getSlot(int x, int y);
    void addObject(std::unique_ptr<Cell> obj, int x, int y);
    void removeTankAt(int x, int y);
    void removeWallAt(int x, int y);
    void removeMineAt(int x, int y);
    void removeShell(Shell* shell, int x, int y);
    void addShell(std::unique_ptr<Shell> shell);
    void removeShellPointerOnly(Shell* shell, const Position& pos);
    bool isPassable(int x, int y) const;
    const std::vector<Shell*>& getShells() const { return shells; }
    void moveShellTo(Shell* shell, int oldX, int oldY, int newX, int newY);
    int getWidth() const;
    int getHeight() const;
    const std::vector<Tank*>& getTanks(int playerId) const;
    void moveTank(Tank* tank, const Position& newPos);
};

#endif