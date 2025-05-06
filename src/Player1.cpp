#include "Player1.h"
#include "MySatelliteView.h"
#include "MyBattleInfo.h"
#include "MyTankAlgorithm.h"

Player1::Player1(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
    : Player(player_index, x, y, max_steps, num_shells),
      boardWidth(x), boardHeight(y), isFirstTurn(true), cachedInfo(MyBattleInfo()) {}

void Player1::updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) {
    auto& myTank = dynamic_cast<MyTankAlgorithm&>(tank);
    Position currentPos(-1,-1);
    if (isFirstTurn) {
        for (size_t j = 0; j < boardHeight; ++j) {
            for (size_t i = 0; i < boardWidth; ++i) {
                char symbol = satellite_view.getObjectAt(i, j);
                Position pos = Position(i,j);
                if (symbol == '%'){
                    currentPos = pos;
                }
                cachedInfo.addObject(pos, symbol, 1, boardHeight, boardWidth);
            }
        }
        isFirstTurn = false;
    } else {
        Position oldPos = cachedInfo.getSelfPosition();
        char symbol = satellite_view.getObjectAt(oldPos.getx(), oldPos.gety());
        cachedInfo.addObject(oldPos, symbol, 1, boardHeight, boardWidth);
    }

    cachedInfo.setSelfPosition(currentPos);
    cachedInfo.addObject(currentPos, '%', 1, boardHeight, boardWidth);

    myTank.updateBattleInfo(cachedInfo);
}
