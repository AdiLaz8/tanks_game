#include <iostream>
#include <dlfcn.h>
#include <vector>

// Function pointer types for extern C factory registry getters
using GameManagerFactoryRegistryPtr = std::vector<void*>*(*)();
using PlayerFactoryRegistryPtr = std::vector<void*>*(*)();
using TankAlgorithmFactoryRegistryPtr = std::vector<void*>*(*)();

int main() {
    std::cout << "=== FINAL REGISTRATION TEST ===" << std::endl;
    
    // Load GameManager library
    void* gameManagerHandle = dlopen("GameManager/GameManager_318772340_206580102.so", RTLD_LAZY | RTLD_GLOBAL);
    if (!gameManagerHandle) {
        std::cerr << "Failed to load GameManager library: " << dlerror() << std::endl;
        return 1;
    }
    
    // Load Algorithm library
    void* algorithmHandle = dlopen("Algorithm/Algorithm_318772340_206580102.so", RTLD_LAZY | RTLD_GLOBAL);
    if (!algorithmHandle) {
        std::cerr << "Failed to load Algorithm library: " << dlerror() << std::endl;
        dlclose(gameManagerHandle);
        return 1;
    }
    
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
    
    // Check registries and count valid entries
    auto* gameManagerFactories = getGameManagerFactoryRegistry();
    auto* playerFactories = getPlayerFactoryRegistry();
    auto* tankAlgorithmFactories = getTankAlgorithmFactoryRegistry();
    
    int validGameManagerFactories = 0;
    int validPlayerFactories = 0;
    int validTankAlgorithmFactories = 0;
    
    for (size_t i = 0; i < gameManagerFactories->size(); i++) {
        if ((*gameManagerFactories)[i] != nullptr) validGameManagerFactories++;
    }
    
    for (size_t i = 0; i < playerFactories->size(); i++) {
        if ((*playerFactories)[i] != nullptr) validPlayerFactories++;
    }
    
    for (size_t i = 0; i < tankAlgorithmFactories->size(); i++) {
        if ((*tankAlgorithmFactories)[i] != nullptr) validTankAlgorithmFactories++;
    }
    
    std::cout << "\n=== REGISTRATION SUMMARY ===" << std::endl;
    std::cout << "GameManager Factory Registry: " << validGameManagerFactories << " valid entries (out of " << gameManagerFactories->size() << " total)" << std::endl;
    std::cout << "Player Factory Registry: " << validPlayerFactories << " valid entries (out of " << playerFactories->size() << " total)" << std::endl;
    std::cout << "TankAlgorithm Factory Registry: " << validTankAlgorithmFactories << " valid entries (out of " << tankAlgorithmFactories->size() << " total)" << std::endl;
    
    // Verify expected registrations
    bool allGood = true;
    
    if (validGameManagerFactories != 1) {
        std::cout << "❌ GameManager factory registry issue!" << std::endl;
        allGood = false;
    } else {
        std::cout << "✅ GameManager factory registry OK" << std::endl;
    }
    
    if (validPlayerFactories != 1) {
        std::cout << "❌ Player factory registry issue!" << std::endl;
        allGood = false;
    } else {
        std::cout << "✅ Player factory registry OK" << std::endl;
    }
    
    if (validTankAlgorithmFactories != 1) {
        std::cout << "❌ TankAlgorithm factory registry issue!" << std::endl;
        allGood = false;
    } else {
        std::cout << "✅ TankAlgorithm factory registry OK" << std::endl;
    }
    
    // Cleanup
    dlclose(algorithmHandle);
    dlclose(gameManagerHandle);
    
    if (allGood) {
        std::cout << "\n🎉 REGISTRATION SYSTEM WORKING CORRECTLY!" << std::endl;
        std::cout << "✅ .so libraries created with correct names:" << std::endl;
        std::cout << "  - GameManager_318772340_206580102.so" << std::endl;
        std::cout << "  - Algorithm_318772340_206580102.so" << std::endl;
        std::cout << "✅ Each registry has exactly 1 valid factory function" << std::endl;
        std::cout << "✅ Ready for production use!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ Registration issues found!" << std::endl;
        return 1;
    }
} 