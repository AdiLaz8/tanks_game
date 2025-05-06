#include "Player2.h"
#include "MyTankAlgorithm.h"
#include <algorithm>

Player2::Player2(int player_index, size_t x, size_t y,
                 size_t max_steps, size_t num_shells)
    : Player(player_index, x, y, max_steps, num_shells),
      playerId(player_index),
      boardRows(y),
      boardCols(x) {}

std::tuple<int, Position> Player2::countAliveTanksAndFindSelf(SatelliteView& view) const {
    int count = 0;
    Position selfPos(-1, -1);

    for (size_t y = 0; y < boardRows; ++y) {
        for (size_t x = 0; x < boardCols; ++x) {
            char c = view.getObjectAt(x, y);
            if (c == '2') {
                count++;
            } else if (c == '%') {
                count++; // גם זה טנק חי שלנו
                selfPos = Position(x, y);
            }
        }
    }

    return {count, selfPos};
}



void Player2::updateTankWithBattleInfo(TankAlgorithm& tank,
                                       SatelliteView& satellite_view) {
    auto& myTank = dynamic_cast<MyTankAlgorithm&>(tank);

    int tankIndex = myTank.getTankIndex();
    MyBattleInfo battleInfo;

    // קבלת כיוון מהטנק
    Direction dir = myTank.getDirection();

    // ניהול מיקום
    Position currentPos = Position(-1,-1);
    if (isFirstTurn) {
        if (currentTankInTurn == 1) {
            std::tie(aliveTanks, currentPos) = countAliveTanksAndFindSelf(satellite_view);
            tankPositions[tankIndex] = currentPos;
        } else {
            // טנקים אחרים בתור הראשון
            for (size_t j = 0; j < boardRows; ++j) {
                for (size_t i = 0; i < boardCols; ++i) {
                    if (satellite_view.getObjectAt(i, j) == '%') {
                        currentPos = Position(i, j);
                        tankPositions[tankIndex] = currentPos;
                        currentTankInTurn++;
                        goto found;
                    }
                }
            }
        }
    found:;
    if(currentTankInTurn > aliveTanks){
        isFirstTurn = false;
        currentTankInTurn = 1;
        }
    }
    else {
        Position dirVector = dir.toVector();
        Position oppositeDirVector = dir.getOppositeDirection().toVector();
        Position prev = tankPositions[tankIndex];
        Position forward = Position((prev.getx() + dirVector.getx() + boardCols) % boardCols, (prev.gety() + dirVector.gety() + boardRows) % boardRows);
        Position backward = Position((prev.getx() + oppositeDirVector.getx() + boardCols) % boardCols, (prev.gety() + oppositeDirVector.gety() + boardRows) % boardRows);
        std::vector<Position> candidates = {prev, forward, backward};
        for (const Position& pos : candidates) {
            char obj = satellite_view.getObjectAt(pos.getx(), pos.gety());
            if (obj == '%') {
                currentPos = pos;
                tankPositions[tankIndex] = pos;
                break;
            }
        }
    }

    // עדכון הקרב
    battleInfo.setSelfDirection(dir);
    battleInfo.setSelfPosition(currentPos);
    battleInfo.addObject(currentPos, '%', playerId, boardRows, boardCols);

    // סביבת 3x3
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            Position neighbor((currentPos.getx() + dx + boardCols) % boardCols, (currentPos.gety() + dy + boardRows) % boardRows);
            char symbol = satellite_view.getObjectAt(neighbor.getx(), neighbor.gety());
            battleInfo.addObject(neighbor, symbol, playerId, boardRows, boardCols);
        }
    }

    // קו ראייה
    Position delta = dir.toVector();
    Position curr = currentPos + Position(0,0);
    curr.move(dir, boardCols, boardRows);
    while (!(curr == currentPos)) {
        char symbol = satellite_view.getObjectAt(curr.getx(), curr.gety());
        battleInfo.addObject(curr, symbol, playerId, boardRows, boardCols);
        curr.move(dir, boardCols, boardRows);
    }

    // סיום
    myTank.updateBattleInfo(battleInfo);
    isFirstTurn = false;
}
