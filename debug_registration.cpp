#include <iostream>
#include <dlfcn.h>
#include <vector>

// Function pointer types for extern C factory registry getters
using GameManagerFactoryRegistryPtr = std::vector<void*>*(*)();
using PlayerFactoryRegistryPtr = std::vector<void*>*(*)();
using TankAlgorithmFactoryRegistryPtr = std::vector<void*>*(*)();

int main() {
    std::cout << "=== DEBUGGING REGISTRATION SYSTEM ===" << std::endl;
    
    // Load GameManager library
    void* gameManagerHandle = dlopen("GameManager/GameManager_318772340_206580102.so", RTLD_LAZY | RTLD_GLOBAL);
    if (!gameManagerHandle) {
        std::cerr << "Failed to load GameManager library: " << dlerror() << std::endl;
        return 1;
    }
    std::cout << "✅ GameManager library loaded" << std::endl;
    
    // Load Algorithm library
    void* algorithmHandle = dlopen("Algorithm/Algorithm_318772340_206580102.so", RTLD_LAZY | RTLD_GLOBAL);
    if (!algorithmHandle) {
        std::cerr << "Failed to load Algorithm library: " << dlerror() << std::endl;
        dlclose(gameManagerHandle);
        return 1;
    }
    std::cout << "✅ Algorithm library loaded" << std::endl;
    
    // Get factory registry functions
    auto getGameManagerFactoryRegistry = (GameManagerFactoryRegistryPtr)dlsym(gameManagerHandle, "getGameManagerFactoryRegistryC");
    auto getPlayerFactoryRegistry = (PlayerFactoryRegistryPtr)dlsym(algorithmHandle, "getPlayerFactoryRegistryC");
    auto getTankAlgorithmFactoryRegistry = (TankAlgorithmFactoryRegistryPtr)dlsym(algorithmHandle, "getTankAlgorithmFactoryRegistryC");
    
    if (!getGameManagerFactoryRegistry || !getPlayerFactoryRegistry || !getTankAlgorithmFactoryRegistry) {
        std::cerr << "Failed to get factory registry functions: " << dlerror() << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    // Check registries multiple times to see if they grow
    std::cout << "\n=== CHECKING FOR MULTIPLE REGISTRATIONS ===" << std::endl;
    
    for (int i = 0; i < 3; i++) {
        auto* gameManagerFactories = getGameManagerFactoryRegistry();
        auto* playerFactories = getPlayerFactoryRegistry();
        auto* tankAlgorithmFactories = getTankAlgorithmFactoryRegistry();
        
        std::cout << "Check " << (i+1) << ":" << std::endl;
        std::cout << "  GameManager Factory Registry: " << gameManagerFactories->size() << " entries" << std::endl;
        std::cout << "  Player Factory Registry: " << playerFactories->size() << " entries" << std::endl;
        std::cout << "  TankAlgorithm Factory Registry: " << tankAlgorithmFactories->size() << " entries" << std::endl;
        
        // Check if entries are unique by printing addresses
        std::cout << "  GameManager factory addresses: ";
        for (size_t j = 0; j < gameManagerFactories->size(); j++) {
            std::cout << (*gameManagerFactories)[j] << " ";
        }
        std::cout << std::endl;
        
        std::cout << "  Player factory addresses: ";
        for (size_t j = 0; j < playerFactories->size(); j++) {
            std::cout << (*playerFactories)[j] << " ";
        }
        std::cout << std::endl;
        
        std::cout << "  TankAlgorithm factory addresses: ";
        for (size_t j = 0; j < tankAlgorithmFactories->size(); j++) {
            std::cout << (*tankAlgorithmFactories)[j] << " ";
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
    
    // Cleanup
    dlclose(algorithmHandle);
    dlclose(gameManagerHandle);
    
    std::cout << "=== DEBUG COMPLETE ===" << std::endl;
    return 0;
} 