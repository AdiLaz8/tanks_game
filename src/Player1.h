#ifndef PLAYER1_H
#define PLAYER1_H

#include "Player.h"
#include <vector>
#include <memory>

class Player1 : public Player {
private:
    int playerIndex;
    size_t maxSteps;
    size_t numShells;
    // אפשר להוסיף כאן נתונים נוספים כמו מיקומים של טנקים

public:
    Player1(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells);

    void updateTankWithBattleInfo(
        TankAlgorithm& tank,
        SatelliteView& satellite_view) override;
};

#endif // PLAYER1_H