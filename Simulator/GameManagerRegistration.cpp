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

// Implementation of GameManagerRegistration constructor
GameManagerRegistration::GameManagerRegistration(GameManagerFactory factory) {
    getGameManagerFactoryRegistry().push_back(factory);
}