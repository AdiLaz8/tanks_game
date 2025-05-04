#include "Player2.h"
#include "Direction.h"
#include "Position.h"
#include <algorithm>

Player2::Player2(int player_index, size_t x, size_t y,
                 size_t max_steps, size_t num_shells)
    : playerId(player_index), boardRows(y), boardCols(x), max_steps(max_steps), num_shells(num_shells) {}

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
            Position neighbor(selfPos.x + dx, selfPos.y + dy);
            int wrappedX = (neighbor.x + boardCols) % boardCols;
            int wrappedY = (neighbor.y + boardRows) % boardRows;
            Position wrapped(wrappedX, wrappedY);
            char symbol = satellite_view.getObjectAt(wrapped.x, wrapped.y);
            battleInfo.addObject(wrapped, symbol, playerId, boardRows, boardCols);
        }
    }

    // הוספת קו ראיה קדימה
    Position delta = selfDir.toVector();
    Position current = selfPos + delta;
    for (int steps = 0; steps < std::max(boardCols, boardRows); ++steps) {
        int wrappedX = (current.x + boardCols) % boardCols;
        int wrappedY = (current.y + boardRows) % boardRows;
        Position wrapped(wrappedX, wrappedY);

        if (wrapped == selfPos)
            break;

        char symbol = satellite_view.getObjectAt(wrapped.x, wrapped.y);
        battleInfo.addObject(wrapped, symbol, playerId, boardRows, boardCols);
        current = current + delta;
    }

    tank.updateBattleInfo(battleInfo);
}
