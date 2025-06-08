#include "MyTankAlgorithmFactory.h"
#include "Algo1.h"
#include "Algo2.h"
std::unique_ptr<TankAlgorithm> MyTankAlgorithmFactory::create(int player_index, int tank_index) const {
    if (player_index == 1)
        return std::make_unique<Algo1>(player_index, tank_index);
    else
        return std::make_unique<Algo2>(player_index, tank_index);
}