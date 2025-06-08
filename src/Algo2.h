#ifndef ALGO2_H
#define ALGO2_H
#include "MyTankAlgorithm.h"
#include <vector>
#include <queue>
#include <unordered_set>
class Algo2 : public MyTankAlgorithm {
public:
    using MyTankAlgorithm::MyTankAlgorithm;
    ActionRequest getAction() override;
    void updateBattleInfo(BattleInfo& info) override;

};
#endif