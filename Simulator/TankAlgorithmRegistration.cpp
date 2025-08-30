#include "../common/TankAlgorithmRegistration.h"
#include "../common/TankAlgorithm.h"
#include "Registry.h"
#include <memory>
#include <vector>

TankAlgorithmRegistration::TankAlgorithmRegistration(TankAlgorithmFactory factory) {
    registerTankAlgorithmFactory(factory);
}