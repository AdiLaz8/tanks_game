#include "Player2.h"
#include "Direction.h"
#include "Position.h"
#include <algorithm>

Player2::Player2(int player_index, size_t x, size_t y,
                 size_t max_steps, size_t num_shells)
    : Player(player_index, x, y, max_steps, num_shells),
      playerId(player_index),
      boardRows(y),
      boardCols(x),
      max_steps(max_steps),
      num_shells(num_shells) {}

void Player2::updateTankWithBattleInfo(TankAlgorithm& tank,
                                       SatelliteView& satellite_view) {
    MyBattleInfo battleInfo;
    MySatelliteView* mySat = dynamic_cast<MySatelliteView*>(&satellite_view);
    if (!mySat) return;

    Position selfPos = mySat->getPosition();
    Direction selfDir = mySat->getTankDirection();

    battleInfo.setSelfDirection(selfDir);
    battleInfo.addObject(selfPos, '%', playerId, boardRows, boardCols);

    // הוספת 3x3 סביבת טנק
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            Position neighbor(selfPos.getx() + dx, selfPos.gety() + dy);
            int wrappedX = (neighbor.getx() + boardCols) % boardCols;
            int wrappedY = (neighbor.gety() + boardRows) % boardRows;
            Position wrapped(wrappedX, wrappedY);
            char symbol = satellite_view.getObjectAt(wrapped.getx(), wrapped.gety());
            battleInfo.addObject(wrapped, symbol, playerId, boardRows, boardCols);
        }
    }

    // הוספת קו ראיה קדימה
    Position delta = selfDir.toVector();
    Position current = selfPos + delta;
    for (int steps = 0; steps < std::max(boardCols, boardRows); ++steps) {
        int wrappedX = (current.getx() + boardCols) % boardCols;
        int wrappedY = (current.gety() + boardRows) % boardRows;
        Position wrapped(wrappedX, wrappedY);

        if (wrapped == selfPos)
            break;

        char symbol = satellite_view.getObjectAt(wrapped.getx(), wrapped.gety());
        battleInfo.addObject(wrapped, symbol, playerId, boardRows, boardCols);
        current = current + delta;
    }

    tank.updateBattleInfo(battleInfo);
}
