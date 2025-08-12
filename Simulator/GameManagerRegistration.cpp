#include "../common/GameManagerRegistration.h"
#include "../common/AbstractGameManager.h"
#include "Registry.h"
#include <memory>
#include <vector>

// Implementation of GameManagerRegistration constructor
GameManagerRegistration::GameManagerRegistration(GameManagerFactory factory) {
    // Use the global registry from Registry.cpp
    registerGameManagerFactory(factory);
}