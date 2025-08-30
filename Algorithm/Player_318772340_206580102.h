#ifndef ALGORITHM_318772340_206580102_MY_PLAYER_H
#define ALGORITHM_318772340_206580102_MY_PLAYER_H

#include "../common/Player.h"
#include "MyBattleInfo.h"
#include "../UserCommon/Position.h"
#include <unordered_map>
#include <unordered_set>

using namespace UserCommon_318772340_206580102;

namespace Algorithm_318772340_206580102 {

class Player_318772340_206580102 : public Player {
public:
    Player_318772340_206580102(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells);
    void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) override;

private:
    int playerIndex;
    size_t boardWidth;
    size_t boardHeight;
    size_t maxSteps;
    size_t numShells;
    
    bool minesInitialized = false;
    std::vector<Position> minePositions;
    
    // Player-specific behavior methods
    void updatePlayer1BattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view);
    void updatePlayer2BattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view);
};

} // namespace Algorithm_318772340_206580102

#endif // ALGORITHM_318772340_206580102_MY_PLAYER_H