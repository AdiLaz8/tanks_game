#ifndef SIMULATOR_DYNAMIC_LOADER_H
#define SIMULATOR_DYNAMIC_LOADER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "../common/AbstractGameManager.h"

class DynamicLoader {
private:
    std::vector<void*> loadedLibraries;
    
public:
    ~DynamicLoader();
    
    // Load a shared library (.so file)
    void* loadLibrary(const std::string& soPath);
    
    // Get registered factories from loaded libraries
    std::vector<PlayerFactory> getPlayerFactories() const;
    std::vector<TankAlgorithmFactory> getTankAlgorithmFactories() const; 
    std::vector<GameManagerFactory> getGameManagerFactories() const;
    
    // Get factory names for identification
    std::vector<std::string> getPlayerFactoryNames() const;
    std::vector<std::string> getTankAlgorithmFactoryNames() const;
    std::vector<std::string> getGameManagerFactoryNames() const;
    
    // Load multiple libraries at once
    void loadGameManagerLibraries(const std::vector<std::string>& soPaths);
    void loadAlgorithmLibraries(const std::vector<std::string>& soPaths);
    
    // Cleanup
    void unloadAll();
};

// Access to registration system
std::vector<PlayerFactory>& getPlayerFactoryRegistry();
std::vector<TankAlgorithmFactory>& getTankAlgorithmFactoryRegistry();
std::vector<GameManagerFactory>& getGameManagerFactoryRegistry();

std::vector<std::string>& getPlayerNameRegistry();
std::vector<std::string>& getTankAlgorithmNameRegistry();
std::vector<std::string>& getGameManagerNameRegistry();

#endif // SIMULATOR_DYNAMIC_LOADER_H