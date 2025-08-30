#include "../common/GameManagerRegistration.h"
#include "../common/AbstractGameManager.h"
#include "Registry.h"
#include <memory>
#include <vector>

GameManagerRegistration::GameManagerRegistration(GameManagerFactory factory) {
    registerGameManagerFactory(factory);
}