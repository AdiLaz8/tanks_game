#include "../common/GameManagerRegistration.h"
#include "../common/AbstractGameManager.h"
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <set>

// Global registry for game manager factories
std::vector<GameManagerFactory>& getGameManagerFactoryRegistry() {
    static std::vector<GameManagerFactory> registry;
    return registry;
}

// Track registered class names to prevent duplicates
std::set<std::string>& getRegisteredGameManagerNames() {
    static std::set<std::string> registeredNames;
    return registeredNames;
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
    // Use a simple approach: only register if registry is empty
    auto& registry = getGameManagerFactoryRegistry();
    if (registry.empty()) {
        registry.push_back(factory);
    }
}