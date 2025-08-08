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

// Implementation of TankAlgorithmRegistration constructor
TankAlgorithmRegistration::TankAlgorithmRegistration(TankAlgorithmFactory factory) {
    getTankAlgorithmFactoryRegistry().push_back(factory);
}

// Global registration functions that Algorithm .so files can call
extern "C" {
    void registerTankAlgorithmFactory(TankAlgorithmFactory factory) {
        getTankAlgorithmFactoryRegistry().push_back(factory);
    }
    
    void registerTankAlgorithmName(const char* name) {
        getTankAlgorithmNameRegistry().push_back(std::string(name));
    }
}