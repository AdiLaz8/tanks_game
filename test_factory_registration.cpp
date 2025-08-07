#include <iostream>
#include <dlfcn.h>
#include <vector>

// Function pointer types for extern C factory registry getters
using GameManagerFactoryRegistryPtr = std::vector<void*>*(*)();
using PlayerFactoryRegistryPtr = std::vector<void*>*(*)();
using TankAlgorithmFactoryRegistryPtr = std::vector<void*>*(*)();

int main() {
    std::cout << "Testing Factory Registration System..." << std::endl;
    
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
    
    // Get factory registry functions using extern C names
    auto getGameManagerFactoryRegistry = (GameManagerFactoryRegistryPtr)dlsym(gameManagerHandle, "getGameManagerFactoryRegistryC");
    auto getPlayerFactoryRegistry = (PlayerFactoryRegistryPtr)dlsym(algorithmHandle, "getPlayerFactoryRegistryC");
    auto getTankAlgorithmFactoryRegistry = (TankAlgorithmFactoryRegistryPtr)dlsym(algorithmHandle, "getTankAlgorithmFactoryRegistryC");
    
    if (!getGameManagerFactoryRegistry || !getPlayerFactoryRegistry || !getTankAlgorithmFactoryRegistry) {
        std::cerr << "Failed to get factory registry functions: " << dlerror() << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    // Check factory registries
    auto* gameManagerFactories = getGameManagerFactoryRegistry();
    auto* playerFactories = getPlayerFactoryRegistry();
    auto* tankAlgorithmFactories = getTankAlgorithmFactoryRegistry();
    
    std::cout << "\n=== Factory Registration Results ===" << std::endl;
    std::cout << "GameManager Factory Registry (" << gameManagerFactories->size() << " entries)" << std::endl;
    std::cout << "Player Factory Registry (" << playerFactories->size() << " entries)" << std::endl;
    std::cout << "TankAlgorithm Factory Registry (" << tankAlgorithmFactories->size() << " entries)" << std::endl;
    
    // Verify expected registrations
    bool allGood = true;
    
    if (gameManagerFactories->size() != 1) {
        std::cout << "❌ GameManager factory registry issue!" << std::endl;
        allGood = false;
    } else {
        std::cout << "✅ GameManager factory registry OK" << std::endl;
    }
    
    if (playerFactories->size() != 1) {
        std::cout << "❌ Player factory registry issue!" << std::endl;
        allGood = false;
    } else {
        std::cout << "✅ Player factory registry OK" << std::endl;
    }
    
    if (tankAlgorithmFactories->size() != 1) {
        std::cout << "❌ TankAlgorithm factory registry issue!" << std::endl;
        allGood = false;
    } else {
        std::cout << "✅ TankAlgorithm factory registry OK" << std::endl;
    }
    
    // Cleanup
    dlclose(algorithmHandle);
    dlclose(gameManagerHandle);
    
    if (allGood) {
        std::cout << "\n🎉 All factory registrations working perfectly!" << std::endl;
        std::cout << "✅ .so libraries created with correct names:" << std::endl;
        std::cout << "  - GameManager_318772340_206580102.so" << std::endl;
        std::cout << "  - Algorithm_318772340_206580102.so" << std::endl;
        std::cout << "✅ Registration system working correctly!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ Factory registration issues found!" << std::endl;
        return 1;
    }
} 