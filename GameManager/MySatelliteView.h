#ifndef GAMEMANAGER_318772340_206580102_MYSATELLITEVIEW_H
#define GAMEMANAGER_318772340_206580102_MYSATELLITEVIEW_H

#include "../common/SatelliteView.h"
#include "../UserCommon/Position.h"
#include <vector>

using namespace UserCommon_318772340_206580102;

namespace GameManager_318772340_206580102 {

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

} // namespace GameManager_318772340_206580102

#endif // GAMEMANAGER_318772340_206580102_MYSATELLITEVIEW_H 