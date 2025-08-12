#ifndef ALGORITHM_318772340_206580102_TANK_ALGORITHM_REGISTRATION_H
#define ALGORITHM_318772340_206580102_TANK_ALGORITHM_REGISTRATION_H

#include <memory>
#include <functional>
#include "../common/TankAlgorithm.h"

struct TankAlgorithmRegistration {
  TankAlgorithmRegistration(TankAlgorithmFactory factory);
};

#define REGISTER_TANK_ALGORITHM(class_name) TankAlgorithmRegistration register_me_##class_name ( [](int player_index, int tank_index) { return std::make_unique<class_name>(player_index, tank_index); } );

#endif // ALGORITHM_318772340_206580102_TANK_ALGORITHM_REGISTRATION_H