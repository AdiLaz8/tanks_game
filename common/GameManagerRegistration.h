#include "AbstractGameManager.h"

struct GameManagerRegistration {
  GameManagerRegistration(GameManagerFactory factory);
};

#define REGISTER_GAME_MANAGER(class_name) \
GameManagerRegistration register_me_##class_name \
        ( [] (bool verbose) { return std::make_unique<class_name>(verbose); } );