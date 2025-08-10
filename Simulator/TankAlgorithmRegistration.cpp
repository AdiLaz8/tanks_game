#include "../common/TankAlgorithmRegistration.h"
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

// Export functions with extern C to avoid mangling
extern "C" {
    std::vector<TankAlgorithmFactory>* getTankAlgorithmFactoryRegistryC() {
        return &getTankAlgorithmFactoryRegistry();
    }
    
    std::vector<std::string>* getTankAlgorithmNameRegistryC() {
        return &getTankAlgorithmNameRegistry();
    }
}

// Implementation of TankAlgorithmRegistration constructor
TankAlgorithmRegistration::TankAlgorithmRegistration(TankAlgorithmFactory factory) {
    getTankAlgorithmFactoryRegistry().push_back(factory);
}