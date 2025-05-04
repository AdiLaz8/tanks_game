#ifndef PLAYER2_H
#define PLAYER2_H

#include "Player.h"
#include "MyBattleInfo.h"
#include "MySatelliteView.h"

class Player2: public Player {
private:
    int playerId;
    size_t boardRows;
    size_t boardCols;
    size_t max_steps;
    size_t num_shells;

public:
    Player2(int player_index, size_t x, size_t y,
            size_t max_steps, size_t num_shells);

    void updateTankWithBattleInfo(TankAlgorithm& tank,
                                   SatelliteView& satellite_view) override;
};

#endif // PLAYER2_H
