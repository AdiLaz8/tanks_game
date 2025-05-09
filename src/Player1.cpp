#include "Player1.h"
#include "MyTankAlgorithm.h"
#include "MyBattleInfo.h"

Player1::Player1(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
    : Player(player_index, x, y, max_steps, num_shells),
      playerIndex(player_index),
      boardWidth(x), boardHeight(y), maxSteps(max_steps), numShells(num_shells) {}

void Player1::updateTankWithBattleInfo(
    TankAlgorithm& tank,
    SatelliteView& satellite_view) {

    //auto& myTank = dynamic_cast<MyTankAlgorithm&>(tank);
    Position currentPos(-1, -1);
    MyBattleInfo battleInfo(boardHeight, boardWidth, numShells);

    // סריקה מלאה של הלוח
    for (size_t j = 0; j < boardHeight; ++j) {
        for (size_t i = 0; i < boardWidth; ++i) {
            char symbol = satellite_view.getObjectAt(i, j);
            Position pos(i, j);

            battleInfo.addObject(pos, symbol, playerIndex, boardHeight, boardWidth);
        }
    }

    tank.updateBattleInfo(battleInfo);
}
