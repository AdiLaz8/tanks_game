#include "DynamicLoader.h"
#include <dlfcn.h>
#include <iostream>
#include <stdexcept>

std::vector<PlayerFactory>& getPlayerFactoryRegistry();
std::vector<TankAlgorithmFactory>& getTankAlgorithmFactoryRegistry();
std::vector<GameManagerFactory>& getGameManagerFactoryRegistry();

std::vector<std::string>& getPlayerNameRegistry();
std::vector<std::string>& getTankAlgorithmNameRegistry();
std::vector<std::string>& getGameManagerNameRegistry();

DynamicLoader::~DynamicLoader() {
    unloadAll();
}

void* DynamicLoader::loadLibrary(const std::string& soPath) {
    // Clear previous error
    dlerror();
    
    // Load the shared library
    void* handle = dlopen(soPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        throw std::runtime_error("Cannot load library " + soPath + ": " + dlerror());
    }
    
    std::cout << "Successfully loaded library: " << soPath << std::endl;
    loadedLibraries.push_back(handle);
    return handle;
}

std::vector<PlayerFactory> DynamicLoader::getPlayerFactories() const {
    return getPlayerFactoryRegistry();
}

std::vector<TankAlgorithmFactory> DynamicLoader::getTankAlgorithmFactories() const {
    return getTankAlgorithmFactoryRegistry();
}

std::vector<GameManagerFactory> DynamicLoader::getGameManagerFactories() const {
    return getGameManagerFactoryRegistry();
}

std::vector<std::string> DynamicLoader::getPlayerFactoryNames() const {
    return getPlayerNameRegistry();
}

std::vector<std::string> DynamicLoader::getTankAlgorithmFactoryNames() const {
    return getTankAlgorithmNameRegistry();
}

std::vector<std::string> DynamicLoader::getGameManagerFactoryNames() const {
    return getGameManagerNameRegistry();
}

void DynamicLoader::loadGameManagerLibraries(const std::vector<std::string>& soPaths) {
    std::cout << "Loading GameManager libraries..." << std::endl;
    for (const auto& path : soPaths) {
        try {
            loadLibrary(path);
        } catch (const std::exception& e) {
            std::cerr << "Failed to load GameManager library " << path << ": " << e.what() << std::endl;
            throw;
        }
    }
    std::cout << "Loaded " << getGameManagerFactoryRegistry().size() << " GameManager factories" << std::endl;
}

void DynamicLoader::loadAlgorithmLibraries(const std::vector<std::string>& soPaths) {
    std::cout << "Loading Algorithm libraries..." << std::endl;
    for (const auto& path : soPaths) {
        try {
            loadLibrary(path);
        } catch (const std::exception& e) {
            std::cerr << "Failed to load Algorithm library " << path << ": " << e.what() << std::endl;
            throw;
        }
    }
    std::cout << "Loaded " << getPlayerFactoryRegistry().size() << " Player factories and " 
              << getTankAlgorithmFactoryRegistry().size() << " TankAlgorithm factories" << std::endl;
}

void DynamicLoader::unloadAll() {
    for (void* handle : loadedLibraries) {
        if (handle) {
            dlclose(handle);
        }
    }
    loadedLibraries.clear();
    
    // Clear registries
    getPlayerFactoryRegistry().clear();
    getTankAlgorithmFactoryRegistry().clear();
    getGameManagerFactoryRegistry().clear();
    getPlayerNameRegistry().clear();
    getTankAlgorithmNameRegistry().clear();
    getGameManagerNameRegistry().clear();
}