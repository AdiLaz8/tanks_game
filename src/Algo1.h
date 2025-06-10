
#ifndef ALGO1_H
#define ALGO1_H

#include "MyTankAlgorithm.h"
#include <vector>
#include <queue>
#include <unordered_set>
class Algo1 : public MyTankAlgorithm {
public:
    using MyTankAlgorithm::MyTankAlgorithm;
    ActionRequest getAction() override;
    void updateBattleInfo(BattleInfo& info) override;

private:
    std::vector<Direction::Value> currentPath;
    Position targetPos{0,0};
    bool chasing = false;
    bool needsNewInfo = true;
    bool needsNewBattleInfo() const;
    void computeShootingPath();
    int check=0;
    std::vector<Direction::Value> computeBFS(const Position& from, const Position& to,
                                             const std::vector<std::pair<Position, char>>& fullView,
                                             size_t width, size_t height);
    ActionRequest getShootingActionIfAvailable();
    ActionRequest moveForwardAfterRotate();
};
#endif

