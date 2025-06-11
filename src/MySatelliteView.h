#ifndef MYSATELLITEVIEW_H
#define MYSATELLITEVIEW_H
#include "SatelliteView.h"
#include "Board.h"
#include "Position.h"

class MySatelliteView : public SatelliteView {
private:
    std::vector<std::vector<char>> boardMatrix;
    Position currentTankPosition;
    size_t width;
    size_t height;

public:
    MySatelliteView(const std::vector<std::vector<char>>& boardMatrix);
    void setPosition(const Position& pos);
    Position getPosition() const;
    char getObjectAt(size_t x, size_t y) const override;
};

#endif 