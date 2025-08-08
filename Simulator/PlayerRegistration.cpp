#include "../common/PlayerRegistration.h"
#include "../common/Player.h"
#include <memory>
#include <vector>

// Global registry for player factories
std::vector<PlayerFactory>& getPlayerFactoryRegistry() {
    static std::vector<PlayerFactory> registry;
    return registry;
}

std::vector<std::string>& getPlayerNameRegistry() {
    static std::vector<std::string> registry;
    return registry;
}

// Implementation of PlayerRegistration constructor
PlayerRegistration::PlayerRegistration(PlayerFactory factory) {
    getPlayerFactoryRegistry().push_back(factory);
}

// Global registration functions that Algorithm .so files can call
extern "C" {
    void registerPlayerFactory(PlayerFactory factory) {
        getPlayerFactoryRegistry().push_back(factory);
    }
    
    void registerPlayerName(const char* name) {
        getPlayerNameRegistry().push_back(std::string(name));
    }
}