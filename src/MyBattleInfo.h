#pragma once

#include "BattleInfo.h"
#include "Position.h"  // אם Position ממומש אצלך
#include <optional>

class MyBattleInfo : public BattleInfo {
public:
    // מיקום עצמי
    void setMyPosition(const Position& pos);
    Position getMyPosition() const;
    bool hasMyPosition() const;

    // תחמושת
    void setShellsLeft(int shells);
    int getShellsLeft() const;

    // האם יש אויב בטווח ראייה (אם החלטת לסמן את זה)
    void setEnemyVisible(bool visible);
    bool isEnemyVisible() const;

    // ניקוי מידע לתחילת תור חדש
    void clear();

private:
    std::optional<Position> myPosition;
    int shellsLeft = 0;
    bool enemyVisible = false;
};
