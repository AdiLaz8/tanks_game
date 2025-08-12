#ifndef SIMULATOR_REGISTRY_H
#define SIMULATOR_REGISTRY_H

#include <vector>
#include <functional>
#include <memory>
#include <string>
#include <mutex>
#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "../common/AbstractGameManager.h"

// Global registries - these will be accessed by the Simulator
extern std::vector<PlayerFactory> playerFactories;
extern std::vector<TankAlgorithmFactory> tankAlgorithmFactories;
extern std::vector<GameManagerFactory> gameManagerFactories;

// Name registries for identification
extern std::vector<std::string> playerNames;
extern std::vector<std::string> tankAlgorithmNames;
extern std::vector<std::string> gameManagerNames;

extern std::mutex registryMutex;

// Registry access functions
std::vector<PlayerFactory>& getPlayerFactoryRegistry();
std::vector<TankAlgorithmFactory>& getTankAlgorithmFactoryRegistry();
std::vector<GameManagerFactory>& getGameManagerFactoryRegistry();

// Name registry access functions
std::vector<std::string>& getPlayerNameRegistry();
std::vector<std::string>& getTankAlgorithmNameRegistry();
std::vector<std::string>& getGameManagerNameRegistry();

// Exported registration functions for dynamically loaded libraries to call
extern "C" {
    void registerPlayerFactory(PlayerFactory factory);
    void registerTankAlgorithmFactory(TankAlgorithmFactory factory);
    void registerGameManagerFactory(GameManagerFactory factory);
    void registerPlayerName(const char* name);
    void registerTankAlgorithmName(const char* name);
    void registerGameManagerName(const char* name);
}

#endif // SIMULATOR_REGISTRY_H


