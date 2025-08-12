#include "../common/TankAlgorithmRegistration.h"
#include "../common/TankAlgorithm.h"
#include "Registry.h"
#include <memory>
#include <vector>

// Implementation of TankAlgorithmRegistration constructor
TankAlgorithmRegistration::TankAlgorithmRegistration(TankAlgorithmFactory factory) {
    // Use the global registry from Registry.cpp
    registerTankAlgorithmFactory(factory);
}