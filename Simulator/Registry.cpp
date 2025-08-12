#include "Registry.h"
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include <iostream>

// Global registries - these will be accessed by the Simulator
std::vector<PlayerFactory> playerFactories;
std::vector<TankAlgorithmFactory> tankAlgorithmFactories;
std::vector<GameManagerFactory> gameManagerFactories;

// Name registries for identification
std::vector<std::string> playerNames;
std::vector<std::string> tankAlgorithmNames;
std::vector<std::string> gameManagerNames;

std::mutex registryMutex;

// Registry access functions
std::vector<PlayerFactory>& getPlayerFactoryRegistry() { return playerFactories; }
std::vector<TankAlgorithmFactory>& getTankAlgorithmFactoryRegistry() { return tankAlgorithmFactories; }
std::vector<GameManagerFactory>& getGameManagerFactoryRegistry() { return gameManagerFactories; }

// Name registry access functions
std::vector<std::string>& getPlayerNameRegistry() { return playerNames; }
std::vector<std::string>& getTankAlgorithmNameRegistry() { return tankAlgorithmNames; }
std::vector<std::string>& getGameManagerNameRegistry() { return gameManagerNames; }

// Exported registration functions for dynamically loaded libraries to call
extern "C" {
    void registerPlayerFactory(PlayerFactory factory) {
        std::lock_guard<std::mutex> lock(registryMutex);
        playerFactories.push_back(factory);
        std::cout << "Registered Player factory, total: " << playerFactories.size() << std::endl;
    }

    void registerTankAlgorithmFactory(TankAlgorithmFactory factory) {
        std::lock_guard<std::mutex> lock(registryMutex);
        tankAlgorithmFactories.push_back(factory);
        std::cout << "Registered TankAlgorithm factory, total: " << tankAlgorithmFactories.size() << std::endl;
    }

    void registerGameManagerFactory(GameManagerFactory factory) {
        std::lock_guard<std::mutex> lock(registryMutex);
        gameManagerFactories.push_back(factory);
        std::cout << "Registered GameManager factory, total: " << gameManagerFactories.size() << std::endl;
    }

    void registerPlayerName(const char* name) {
        std::lock_guard<std::mutex> lock(registryMutex);
        playerNames.push_back(std::string(name));
        std::cout << "Registered Player name: " << name << std::endl;
    }

    void registerTankAlgorithmName(const char* name) {
        std::lock_guard<std::mutex> lock(registryMutex);
        tankAlgorithmNames.push_back(std::string(name));
        std::cout << "Registered TankAlgorithm name: " << name << std::endl;
    }

    void registerGameManagerName(const char* name) {
        std::lock_guard<std::mutex> lock(registryMutex);
        gameManagerNames.push_back(std::string(name));
        std::cout << "Registered GameManager name: " << name << std::endl;
    }
}


