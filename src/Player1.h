#ifndef PLAYER1_H
#define PLAYER1_H
#include "Player.h"
#include "MyBattleInfo.h"
#include <unordered_map>
class Player1 : public Player {
public:
    Player1(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells);
    void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) override;

private:
    int playerIndex;
    size_t boardWidth;
    size_t boardHeight;
    size_t maxSteps;
    size_t numShells;
};
#endif 
