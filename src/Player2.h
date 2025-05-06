#ifndef PLAYER2_H
#define PLAYER2_H

#include "Player.h"
#include "MyBattleInfo.h"
#include "MySatelliteView.h"
#include <unordered_map>

class Player2: public Player {
private:
    int playerId;
    size_t boardRows;
    size_t boardCols;
    size_t max_steps;
    size_t num_shells;
    bool isFirstTurn = true;
    int currentTankInTurn = 1;
    int aliveTanks = -1;
    std::unordered_map<int, Position> tankPositions;  // tankIndex → Position
    std::unordered_map<int, Direction> tankDirections; // tankIndex → Direction
    std::tuple<int, Position> countAliveTanksAndFindSelf(SatelliteView& view) const;




public:
    Player2(int player_index, size_t x, size_t y,
            size_t max_steps, size_t num_shells);

    void updateTankWithBattleInfo(TankAlgorithm& tank,
                                   SatelliteView& satellite_view) override;
};

#endif // PLAYER2_H
