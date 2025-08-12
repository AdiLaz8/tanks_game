#include "../common/PlayerRegistration.h"
#include "../common/Player.h"
#include "Registry.h"
#include <memory>
#include <vector>

// Implementation of PlayerRegistration constructor
PlayerRegistration::PlayerRegistration(PlayerFactory factory) {
    // Use the global registry from Registry.cpp
    registerPlayerFactory(factory);
}