#ifndef MYSATELLITEVIEW_H
#define MYSATELLITEVIEW_H

#include "SatelliteView.h"
#include "Board.h"
#include "Position.h"

class MySatelliteView : public SatelliteView {
private:
    const Board& board;
    Position currentTankPosition;
    size_t width;
    size_t height;

public:
    MySatelliteView(const Board& board)
        : board(board), width(board.getWidth()), height(board.getHeight()) {}

    void setPosition(const Position& pos) {
        currentTankPosition = pos;
    }

    Position getPosition() const {
        return currentTankPosition;
    }

    char getObjectAt(size_t x, size_t y) const override {
        if (x >= width || y >= height)
            return '&';

        const CellSlot& slot = board.getSlot(x, y);
        
        if (!slot.getShells().empty())
            return '*';
        if (slot.getTank())
            return slot.getTank()->getSymbol();
        if (slot.getMine())
            return '@';
        if (slot.getWall())
            return '#';
        return ' ';
    }
};

#endif // MYSATELLITEVIEW_H
