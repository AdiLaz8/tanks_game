#include <iostream>
#include <dlfcn.h>
#include <vector>
#include <string>

// Function pointer types for registry getters
using GameManagerFactoryRegistry = std::vector<std::string>&(*)();
using PlayerFactoryRegistry = std::vector<std::string>&(*)();
using TankAlgorithmFactoryRegistry = std::vector<std::string>&(*)();

int main() {
    std::cout << "Testing Registration System..." << std::endl;
    
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
    
    // Get GameManager registry
    auto getGameManagerRegistry = (GameManagerFactoryRegistry)dlsym(gameManagerHandle, "_Z29getGameManagerFactoryRegistryv");
    if (!getGameManagerRegistry) {
        std::cerr << "Failed to get GameManager registry function: " << dlerror() << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    // Get Player registry
    auto getPlayerRegistry = (PlayerFactoryRegistry)dlsym(algorithmHandle, "_Z24getPlayerFactoryRegistryv");
    if (!getPlayerRegistry) {
        std::cerr << "Failed to get Player registry function: " << dlerror() << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    // Get TankAlgorithm registry
    auto getTankAlgorithmRegistry = (TankAlgorithmFactoryRegistry)dlsym(algorithmHandle, "_Z31getTankAlgorithmFactoryRegistryv");
    if (!getTankAlgorithmRegistry) {
        std::cerr << "Failed to get TankAlgorithm registry function: " << dlerror() << std::endl;
        dlclose(algorithmHandle);
        dlclose(gameManagerHandle);
        return 1;
    }
    
    // Check registries
    auto& gameManagerRegistry = getGameManagerRegistry();
    auto& playerRegistry = getPlayerRegistry();
    auto& tankAlgorithmRegistry = getTankAlgorithmRegistry();
    
    std::cout << "\n=== Registration Results ===" << std::endl;
    std::cout << "GameManager Registry (" << gameManagerRegistry.size() << " entries):" << std::endl;
    for (const auto& name : gameManagerRegistry) {
        std::cout << "  - " << name << std::endl;
    }
    
    std::cout << "\nPlayer Registry (" << playerRegistry.size() << " entries):" << std::endl;
    for (const auto& name : playerRegistry) {
        std::cout << "  - " << name << std::endl;
    }
    
    std::cout << "\nTankAlgorithm Registry (" << tankAlgorithmRegistry.size() << " entries):" << std::endl;
    for (const auto& name : tankAlgorithmRegistry) {
        std::cout << "  - " << name << std::endl;
    }
    
    // Verify expected registrations
    bool allGood = true;
    
    if (gameManagerRegistry.size() != 1 || gameManagerRegistry[0] != "GameManager_318772340_206580102") {
        std::cout << "❌ GameManager registry issue!" << std::endl;
        allGood = false;
    } else {
        std::cout << "✅ GameManager registry OK" << std::endl;
    }
    
    if (playerRegistry.size() != 1 || playerRegistry[0] != "Player_318772340_206580102") {
        std::cout << "❌ Player registry issue!" << std::endl;
        allGood = false;
    } else {
        std::cout << "✅ Player registry OK" << std::endl;
    }
    
    if (tankAlgorithmRegistry.size() != 1 || tankAlgorithmRegistry[0] != "TankAlgorithm_318772340_206580102") {
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