#include "MyBattleInfo.h"
#include <set>
void MyBattleInfo::reset() {
    fullView.clear();
    directionalView.clear();
    shellPositions.clear();
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j)
            localView[i][j] = ' ';
    selfPositionSet = false;
}
MyBattleInfo::MyBattleInfo(size_t rows, size_t cols) : boardRows(rows), boardCols(cols) {
    reset();
}


void MyBattleInfo::addObject(const Position& pos, char symbol, int playerId, int boardRows, int boardCols) {
    if (symbol == '*')
        shellPositions.push_back(pos);

    if (symbol == '%') {
        selfPosition = pos;
        selfPositionSet = true;
    }

    if (playerId == 1)
        fullView.emplace_back(pos, symbol);

    if (!selfPositionSet)
        return;

    // רשימת המשבצות הספציפיות לבדיקת שכנים מורחבת
    const std::vector<std::pair<int, int>> offsets = {
        {1, 1}, {-1, -1}, {1, 0}, {2, 0}, {2, 2}, {-2, -2},
        {0, -1}, {0, -2}, {-1, 0}, {-2, 0}, {-1, 1}, {-2, 2},
        {0, 1}, {0, 2}, {1, -1}, {2, -2}
    };

    // סט למניעת כפילות
    std::set<Position> seenPositions;

    for (const auto& [dx, dy] : offsets) {
        int wrappedX = (selfPosition.getx() + dx + boardCols) % boardCols;
        int wrappedY = (selfPosition.gety() + dy + boardRows) % boardRows;
        Position neighbor(wrappedX, wrappedY);

        // וידוא שזה לא המיקום העצמי שלנו
        if (neighbor == selfPosition) continue;

        // בדוק אם כבר הוספנו את המשבצת הזו
        if (seenPositions.count(neighbor) == 0) {
            seenPositions.insert(neighbor);

            // המרה למיקום במטריצת 5x5
            int localX = dx + 2;
            int localY = dy + 2;

            // ודא שהמיקום תקין בתוך המערך
            if (localX >= 0 && localX < 5 && localY >= 0 && localY < 5) {
                localView[localX][localY] = symbol;
            }
        }
    }


    // ניהול directionalView
    Position delta = selfDirection.toVector();
    Position current = selfPosition + delta;

    // המשך להתקדם בכיוון כל עוד לא חזרת למיקום ההתחלתי
    while (true) {
        // טיפול ב-wraparound
        current.setx((current.getx() + boardCols) % boardCols);
        current.sety((current.gety() + boardRows) % boardRows);

        // עצור אם חזרת למיקום ההתחלתי
        if (current == selfPosition)
            break;

        // הוסף את המשבצת הנוכחית ל-directionalView
        directionalView.emplace_back(current, symbol);

        // התקדם למשבצת הבאה בכיוון
        current = current + delta;
    }

}

const std::vector<std::pair<Position, char>>& MyBattleInfo::getFullView() const {
    return fullView;
}

const char (&MyBattleInfo::getLocalView() const)[5][5] {
    return localView;
}

const std::vector<std::pair<Position, char>>& MyBattleInfo::getDirectionalView() const {
    return directionalView;
}

const std::vector<Position>& MyBattleInfo::getShellPositions() const {
    return shellPositions;
}

Position MyBattleInfo::getSelfPosition() const {
    return selfPosition;
}

Direction MyBattleInfo::getSelfDirection() const {
    return selfDirection;
}

void MyBattleInfo::setSelfDirection(Direction d) {
    selfDirection = d;
}

void MyBattleInfo::setSelfPosition(Position p) {
    selfPosition = p;
}