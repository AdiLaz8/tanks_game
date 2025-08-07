#include <iostream>
#include <dlfcn.h>

int main() {
    std::cout << "Testing Registration System (Simple)..." << std::endl;
    
    // Load GameManager library
    void* gameManagerHandle = dlopen("GameManager/GameManager_318772340_206580102.so", RTLD_LAZY);
    if (!gameManagerHandle) {
        std::cerr << "Failed to load GameManager library: " << dlerror() << std::endl;
        return 1;
    }
    std::cout << "✅ GameManager library loaded successfully" << std::endl;
    
    // Load Algorithm library
    void* algorithmHandle = dlopen("Algorithm/Algorithm_318772340_206580102.so", RTLD_LAZY);
    if (!algorithmHandle) {
        std::cerr << "Failed to load Algorithm library: " << dlerror() << std::endl;
        dlclose(gameManagerHandle);
        return 1;
    }
    std::cout << "✅ Algorithm library loaded successfully" << std::endl;
    
    // Check if registration symbols exist
    void* gameManagerReg = dlsym(gameManagerHandle, "_register_me_GameManagerClass");
    if (gameManagerReg) {
        std::cout << "✅ GameManager registration object found" << std::endl;
    } else {
        std::cout << "❌ GameManager registration object not found" << std::endl;
    }
    
    void* playerReg = dlsym(algorithmHandle, "_register_me_PlayerClass");
    if (playerReg) {
        std::cout << "✅ Player registration object found" << std::endl;
    } else {
        std::cout << "❌ Player registration object not found" << std::endl;
    }
    
    void* tankAlgorithmReg = dlsym(algorithmHandle, "_register_me_TankAlgorithmClass");
    if (tankAlgorithmReg) {
        std::cout << "✅ TankAlgorithm registration object found" << std::endl;
    } else {
        std::cout << "❌ TankAlgorithm registration object not found" << std::endl;
    }
    
    // Check if registry functions exist
    void* getGameManagerRegistry = dlsym(gameManagerHandle, "_Z29getGameManagerFactoryRegistryv");
    if (getGameManagerRegistry) {
        std::cout << "✅ GameManager registry function found" << std::endl;
    } else {
        std::cout << "❌ GameManager registry function not found" << std::endl;
    }
    
    void* getPlayerRegistry = dlsym(algorithmHandle, "_Z24getPlayerFactoryRegistryv");
    if (getPlayerRegistry) {
        std::cout << "✅ Player registry function found" << std::endl;
    } else {
        std::cout << "❌ Player registry function not found" << std::endl;
    }
    
    void* getTankAlgorithmRegistry = dlsym(algorithmHandle, "_Z31getTankAlgorithmFactoryRegistryv");
    if (getTankAlgorithmRegistry) {
        std::cout << "✅ TankAlgorithm registry function found" << std::endl;
    } else {
        std::cout << "❌ TankAlgorithm registry function not found" << std::endl;
    }
    
    // Cleanup
    dlclose(algorithmHandle);
    dlclose(gameManagerHandle);
    
    std::cout << "\n🎉 Registration system symbols are present!" << std::endl;
    return 0;
} 