#include "../common/GameManagerRegistration.h"
#include "../common/AbstractGameManager.h"
#include <memory>
#include <vector>

// Global registry for game manager factories
std::vector<GameManagerFactory>& getGameManagerFactoryRegistry() {
    static std::vector<GameManagerFactory> registry;
    return registry;
}

std::vector<std::string>& getGameManagerNameRegistry() {
    static std::vector<std::string> registry;
    return registry;
}

// Export functions with extern C to avoid mangling
extern "C" {
    std::vector<GameManagerFactory>* getGameManagerFactoryRegistryC() {
        return &getGameManagerFactoryRegistry();
    }
    
    std::vector<std::string>* getGameManagerNameRegistryC() {
        return &getGameManagerNameRegistry();
    }
}

// Implementation of GameManagerRegistration constructor
GameManagerRegistration::GameManagerRegistration(GameManagerFactory factory) {
    getGameManagerFactoryRegistry().push_back(factory);
}