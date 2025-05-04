#include "MySatelliteView.h"

MySatelliteView::MySatelliteView(const Board& board)
    : board(board),
      currentTankPosition(Position(-1, -1)),
      width(board.getWidth()),
      height(board.getHeight()) {}

void MySatelliteView::setPosition(const Position& pos) {
    currentTankPosition = pos;
}

Position MySatelliteView::getPosition() const {
    return currentTankPosition;
}

char MySatelliteView::getObjectAt(size_t x, size_t y) const {
    if (x >= width || y >= height)
        return '&';
    if (Position(x, y) == currentTankPosition)
        return '%';
    
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
