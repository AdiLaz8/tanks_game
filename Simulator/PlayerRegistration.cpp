#include "../common/PlayerRegistration.h"
#include "../common/Player.h"
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>

// Global registry for player factories
std::vector<PlayerFactory>& getPlayerFactoryRegistry() {
    static std::vector<PlayerFactory> registry;
    return registry;
}

std::vector<std::string>& getPlayerNameRegistry() {
    static std::vector<std::string> registry;
    return registry;
}

// Export functions with extern C to avoid mangling
extern "C" {
    std::vector<PlayerFactory>* getPlayerFactoryRegistryC() {
        return &getPlayerFactoryRegistry();
    }
    
    std::vector<std::string>* getPlayerNameRegistryC() {
        return &getPlayerNameRegistry();
    }
}

// Implementation of PlayerRegistration constructor
PlayerRegistration::PlayerRegistration(PlayerFactory factory) {
    // Use a simple approach: only register if registry is empty
    auto& registry = getPlayerFactoryRegistry();
    if (registry.empty()) {
        registry.push_back(factory);
    }
}