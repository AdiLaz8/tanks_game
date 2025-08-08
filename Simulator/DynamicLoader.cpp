#include "DynamicLoader.h"
#include <dlfcn.h>
#include <iostream>
#include <stdexcept>

// Forward declarations for registration system access
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
    
    // Extract registration objects from the loaded library
    extractRegistrations(handle);
    
    return handle;
}

void DynamicLoader::extractRegistrations(void* handle) {
    dlerror(); // Clear previous error
    
    // Try to find Player registration objects
    typedef PlayerRegistration* (*GetPlayerRegistrationsFunc)();
    GetPlayerRegistrationsFunc getPlayerRegistrations = (GetPlayerRegistrationsFunc)dlsym(handle, "getPlayerRegistrations");
    if (getPlayerRegistrations) {
        PlayerRegistration* registrations = getPlayerRegistrations();
        // Register the factories with the Simulator's registry
        // This is a simplified approach - in practice we'd need to iterate through the registrations
    }
    
    // Try to find TankAlgorithm registration objects
    typedef TankAlgorithmRegistration* (*GetTankAlgorithmRegistrationsFunc)();
    GetTankAlgorithmRegistrationsFunc getTankAlgorithmRegistrations = (GetTankAlgorithmRegistrationsFunc)dlsym(handle, "getTankAlgorithmRegistrations");
    if (getTankAlgorithmRegistrations) {
        TankAlgorithmRegistration* registrations = getTankAlgorithmRegistrations();
        // Register the factories with the Simulator's registry
    }
    
    // Try to find GameManager registration objects
    typedef GameManagerRegistration* (*GetGameManagerRegistrationsFunc)();
    GetGameManagerRegistrationsFunc getGameManagerRegistrations = (GetGameManagerRegistrationsFunc)dlsym(handle, "getGameManagerRegistrations");
    if (getGameManagerRegistrations) {
        GameManagerRegistration* registrations = getGameManagerRegistrations();
        // Register the factories with the Simulator's registry
    }
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