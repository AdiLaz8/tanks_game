#include "MySatelliteView.h"

using namespace UserCommon_318772340_206580102;

namespace GameManager_318772340_206580102 {

MySatelliteView::MySatelliteView(const std::vector<std::vector<char>>& boardMatrix)
    : boardMatrix(boardMatrix),
      currentTankPosition(Position(-1, -1)),
      width(boardMatrix[0].size()),
      height(boardMatrix.size()) {}

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
    return boardMatrix[y][x];
}

} // namespace GameManager_318772340_206580102
