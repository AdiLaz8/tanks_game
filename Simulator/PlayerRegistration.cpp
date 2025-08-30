#include "../common/PlayerRegistration.h"
#include "../common/Player.h"
#include "Registry.h"
#include <memory>
#include <vector>

PlayerRegistration::PlayerRegistration(PlayerFactory factory) {
    registerPlayerFactory(factory);
}