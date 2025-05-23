// #include "Player2.h"
// #include "MyTankAlgorithm.h"
// #include <algorithm>
// #include <set>

// Player2::Player2(int player_index, size_t x, size_t y,
//                  size_t max_steps, size_t num_shells)
//     : Player(player_index, x, y, max_steps, num_shells),
//       playerId(player_index),
//       boardRows(y),
//       boardCols(x) {}

// std::tuple<int, Position> Player2::countAliveTanksAndFindSelf(SatelliteView& view) const {
//     int count = 0;
//     Position selfPos(-1, -1);

//     for (size_t y = 0; y < boardRows; ++y) {
//         for (size_t x = 0; x < boardCols; ++x) {
//             char c = view.getObjectAt(x, y);
//             if (c == '2') {
//                 count++;
//             } else if (c == '%') {
//                 count++; // גם זה טנק חי שלנו
//                 selfPos = Position(x, y);
//             }
//         }
//     }

//     return {count, selfPos};
// }



// void Player2::updateTankWithBattleInfo(TankAlgorithm& tank,
//                                        SatelliteView& satellite_view) {
//     auto& myTank = dynamic_cast<MyTankAlgorithm&>(tank);

//     int tankIndex = myTank.getTankId();
//     MyBattleInfo battleInfo(boardCols,boardRows);

//     // קבלת כיוון מהטנק
//     Direction dir = myTank.getTankDirection();

//     // ניהול מיקום
//     Position currentPos = Position(-1,-1);
//     if (isFirstTurn) {
//         if (currentTankInTurn == 1) {
//             std::tie(aliveTanks, currentPos) = countAliveTanksAndFindSelf(satellite_view);
//             tankPositions[tankIndex] = currentPos;
//         } else {
//             // טנקים אחרים בתור הראשון
//             for (size_t j = 0; j < boardRows; ++j) {
//                 for (size_t i = 0; i < boardCols; ++i) {
//                     if (satellite_view.getObjectAt(i, j) == '%') {
//                         currentPos = Position(i, j);
//                         tankPositions[tankIndex] = currentPos;
//                         currentTankInTurn++;
//                         goto found;
//                     }
//                 }
//             }
//         }
//     found:;
//     if(currentTankInTurn > aliveTanks){
//         isFirstTurn = false;
//         currentTankInTurn = 1;
//         }
//     }
//     else {
//         Position dirVector = dir.toVector();
//         Position oppositeDirVector = dir.getOppositeDirection().toVector();
//         Position prev = tankPositions[tankIndex];
//         Position forward = Position((prev.getx() + dirVector.getx() + boardCols) % boardCols, (prev.gety() + dirVector.gety() + boardRows) % boardRows);
//         Position backward = Position((prev.getx() + oppositeDirVector.getx() + boardCols) % boardCols, (prev.gety() + oppositeDirVector.gety() + boardRows) % boardRows);
//         std::vector<Position> candidates = {prev, forward, backward};
//         for (const Position& pos : candidates) {
//             char obj = satellite_view.getObjectAt(pos.getx(), pos.gety());
//             if (obj == '%') {
//                 currentPos = pos;
//                 tankPositions[tankIndex] = pos;
//                 break;
//             }
//         }
//     }

//     // עדכון הקרב
//     battleInfo.setSelfDirection(dir);
//     battleInfo.setSelfPosition(currentPos);
//     battleInfo.addObject(currentPos, '%', playerId, boardRows, boardCols);

//        // סביבת מורחבת
//     const std::vector<std::pair<int, int>> extendedOffsets = {
//         {1, 1}, {-1, -1}, {1, 0}, {2, 0}, {2, 2}, {-2, -2},
//         {0, -1}, {0, -2}, {-1, 0}, {-2, 0}, {-1, 1}, {-2, 2},
//         {0, 1}, {0, 2}, {1, -1}, {2, -2}
//     };

//     std::set<Position> seenPositions;
//     for (const auto& [dx, dy] : extendedOffsets) {
//         int wrappedX = (currentPos.getx() + dx + boardCols) % boardCols;
//         int wrappedY = (currentPos.gety() + dy + boardRows) % boardRows;
//         Position neighbor(wrappedX, wrappedY);

//         // וידוא שזה לא המיקום העצמי שלנו ושלא כבר הוספנו את המשבצת הזו
//         if (!(neighbor == currentPos) && seenPositions.count(neighbor) == 0) {
//             seenPositions.insert(neighbor);
//             battleInfo.addObject(neighbor, satellite_view.getObjectAt(wrappedX, wrappedY), playerId, boardRows, boardCols);
//         }
//     }

//     // קו ראייה
//     Position delta = dir.toVector();
//     Position curr = currentPos;
//     while (true) {
//         // טיפול ב-wraparound
//         curr.setx((curr.getx() + delta.getx() + boardCols) % boardCols);
//         curr.sety((curr.gety() + delta.gety() + boardRows) % boardRows);

//         // עצור אם חזרת למיקום ההתחלתי
//         if (curr == currentPos)
//             break;

//         // הוסף את המשבצת הנוכחית ל-directionalView
//         battleInfo.addObject(curr, satellite_view.getObjectAt(curr.getx(), curr.gety()), playerId, boardRows, boardCols);
//     }

//     // סיום
//     myTank.updateBattleInfo(battleInfo);
// }

#include "Player2.h"
#include "MyTankAlgorithm.h"
#include "MyBattleInfo.h"

Player2::Player2(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
    : Player(player_index, x, y, max_steps, num_shells),
      playerIndex(player_index),
      boardWidth(x), boardHeight(y), maxSteps(max_steps), numShells(num_shells) {
    (void)player_index; (void)x; (void)y; (void)max_steps; (void)num_shells;
}

void Player2::updateTankWithBattleInfo(
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
