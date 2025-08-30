#include "Player_318772340_206580102.h"
#include "MyBattleInfo.h"
#include "../common/PlayerRegistration.h"

using namespace UserCommon_318772340_206580102;

namespace Algorithm_318772340_206580102 {

Player_318772340_206580102::Player_318772340_206580102(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
    : playerIndex(player_index),
      boardWidth(x), boardHeight(y), maxSteps(max_steps), numShells(num_shells) {
    (void)player_index; (void)x; (void)y; (void)max_steps; (void)num_shells;
}

void Player_318772340_206580102::updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) {
    if (playerIndex == 1) {
        updatePlayer1BattleInfo(tank, satellite_view);
    } else {
        updatePlayer2BattleInfo(tank, satellite_view);
    }
}

void Player_318772340_206580102::updatePlayer1BattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) {
    (void)maxSteps;
    Position currentPos(-1, -1);
    
    // Initialize mines tracking for Player 1
    if (!minesInitialized) {
        minePositions.clear();
        for (size_t j = 0; j < boardHeight; ++j) {
            for (size_t i = 0; i < boardWidth; ++i) {
                char symbol = satellite_view.getObjectAt(i, j);
                if (symbol == '@') {
                    minePositions.emplace_back(i, j);
                }
            }
        }
        minesInitialized = true;
    }
    
    MyBattleInfo battleInfo(boardHeight, boardWidth, numShells);
    battleInfo.setMinesPositions(minePositions);
    
    for (size_t j = 0; j < boardHeight; ++j) {
        for (size_t i = 0; i < boardWidth; ++i) {
            char symbol = satellite_view.getObjectAt(i, j);
            Position pos(i, j);
            battleInfo.addObject(pos, symbol, playerIndex, boardHeight, boardWidth);
        }
    }
    
    tank.updateBattleInfo(battleInfo);
}

void Player_318772340_206580102::updatePlayer2BattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) {
    (void)maxSteps;
    Position currentPos(-1, -1);
    
    MyBattleInfo battleInfo(boardHeight, boardWidth, numShells);
    
    for (size_t j = 0; j < boardHeight; ++j) {
        for (size_t i = 0; i < boardWidth; ++i) {
            char symbol = satellite_view.getObjectAt(i, j);
            Position pos(i, j);
            battleInfo.addObject(pos, symbol, playerIndex, boardHeight, boardWidth);
        }
    }
    
    tank.updateBattleInfo(battleInfo);
}

} // namespace Algorithm_318772340_206580102

// Register the Player (outside namespace)
typedef Algorithm_318772340_206580102::Player_318772340_206580102 PlayerClass;
REGISTER_PLAYER(PlayerClass)
