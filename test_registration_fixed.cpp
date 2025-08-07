#include <iostream>
#include <dlfcn.h>
#include <vector>
#include <string>

// Function pointer types for extern C registry getters
using GameManagerFactoryRegistryPtr = std::vector<void*>*(*)();
using PlayerFactoryRegistryPtr = std::vector<void*>*(*)();
using TankAlgorithmFactoryRegistryPtr = std::vector<void*>*(*)();

using GameManagerNameRegistryPtr = std::vector<std::string>*(*)();
using PlayerNameRegistryPtr = std::vector<std::string>*(*)();
using TankAlgorithmNameRegistryPtr = std::vector<std::string>*(*)();

int main() {
    std::cout << "Testing Registration System (Fixed)..." << std::endl;
    
    // Load GameManager library
    void* gameManagerHandle = dlopen("GameManager/GameManager_318772340_206580102.so", RTLD_LAZY);
    if (!gameManagerHandle) {
        std::cerr << "Failed to load GameManager library: " << dlerror() << std::endl;
        return 1;
    }
    
    // Load Algorithm library
    void* algorithmHandle = dlopen("Algorithm/Algorithm_318772340_206580102.so", RTLD_LAZY);
    if (!algorithmHandle) {
        std::cerr << "Failed to load Algorithm library: " << dlerror() << std::endl;
        dlclose(gameManagerHandle);
        return 1;
    }
    
    // Get registry functions using extern C names
    auto getGameManagerNameRegistry = (GameManagerNameRegistryPtr)dlsym(gameManagerHandle, "getGameManagerNameRegistryC");
    auto getPlayerNameRegistry = (PlayerNameRegistryPtr)dlsym(algorithmHandle, "getPlayerNameRegistryC");
    auto getTankAlgorithmNameRegistry = (TankAlgorithmNameRegistryPtr)dlsym(algorithmHandle, "getTankAlgorithmNameRegistryC");
    
    if (!getGameManagerNameRegistry || !getPlayerNameRegistry || !getTankAlgorithmNameRegistry) {
        std::cerr << "Failed to get registry functions: " << dlerror() << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    // Check registries
    auto* gameManagerNames = getGameManagerNameRegistry();
    auto* playerNames = getPlayerNameRegistry();
    auto* tankAlgorithmNames = getTankAlgorithmNameRegistry();
    
    std::cout << "\n=== Registration Results ===" << std::endl;
    std::cout << "GameManager Registry (" << gameManagerNames->size() << " entries):" << std::endl;
    for (const auto& name : *gameManagerNames) {
        std::cout << "  - " << name << std::endl;
    }
    
    std::cout << "\nPlayer Registry (" << playerNames->size() << " entries):" << std::endl;
    for (const auto& name : *playerNames) {
        std::cout << "  - " << name << std::endl;
    }
    
    std::cout << "\nTankAlgorithm Registry (" << tankAlgorithmNames->size() << " entries):" << std::endl;
    for (const auto& name : *tankAlgorithmNames) {
        std::cout << "  - " << name << std::endl;
    }
    
    // Verify expected registrations
    bool allGood = true;
    
    if (gameManagerNames->size() != 1 || (*gameManagerNames)[0] != "GameManager_318772340_206580102") {
        std::cout << "❌ GameManager registry issue!" << std::endl;
        allGood = false;
    } else {
        std::cout << "✅ GameManager registry OK" << std::endl;
    }
    
    if (playerNames->size() != 1 || (*playerNames)[0] != "Player_318772340_206580102") {
        std::cout << "❌ Player registry issue!" << std::endl;
        allGood = false;
    } else {
        std::cout << "✅ Player registry OK" << std::endl;
    }
    
    if (tankAlgorithmNames->size() != 1 || (*tankAlgorithmNames)[0] != "TankAlgorithm_318772340_206580102") {
        std::cout << "❌ TankAlgorithm registry issue!" << std::endl;
        allGood = false;
    } else {
        std::cout << "✅ TankAlgorithm registry OK" << std::endl;
    }
    
    // Cleanup
    dlclose(algorithmHandle);
    dlclose(gameManagerHandle);
    
    if (allGood) {
        std::cout << "\n🎉 All registrations working perfectly!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ Registration issues found!" << std::endl;
        return 1;
    }
} 