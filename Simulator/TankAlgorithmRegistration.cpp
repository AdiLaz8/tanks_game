#include "../Algorithm/TankAlgorithmRegistration.h"
#include "../common/TankAlgorithm.h"
#include <memory>
#include <vector>

// Global registry for tank algorithm factories
std::vector<TankAlgorithmFactory>& getTankAlgorithmFactoryRegistry() {
    static std::vector<TankAlgorithmFactory> registry;
    return registry;
}

std::vector<std::string>& getTankAlgorithmNameRegistry() {
    static std::vector<std::string> registry;
    return registry;
}

// Implementation of TankAlgorithmRegistration constructor
TankAlgorithmRegistration::TankAlgorithmRegistration(TankAlgorithmFactory factory) {
    getTankAlgorithmFactoryRegistry().push_back(factory);
}