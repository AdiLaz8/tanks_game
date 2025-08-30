#include "ComparativeSimulator.h"
#include "Registry.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <filesystem>
#include <thread>

bool ComparativeGameResult::operator<(const ComparativeGameResult& other) const {
    if (winner != other.winner) return winner < other.winner;
    if (reason != other.reason) return reason < other.reason;
    if (rounds != other.rounds) return rounds < other.rounds;
    if (remaining_tanks != other.remaining_tanks) return remaining_tanks < other.remaining_tanks;
    return finalMapState < other.finalMapState;
}

bool ComparativeGameResult::operator==(const ComparativeGameResult& other) const {
    return winner == other.winner &&
           reason == other.reason &&
           rounds == other.rounds &&
           remaining_tanks == other.remaining_tanks &&
           finalMapState == other.finalMapState;
}

ComparativeSimulator::ComparativeSimulator() = default;
ComparativeSimulator::~ComparativeSimulator() = default;

void ComparativeSimulator::configure(const std::string& gameManagersFolder,
                                   const std::string& gameMapFile,
                                   const std::string& algorithm1File,
                                   const std::string& algorithm2File,
                                   size_t numThreads,
                                   bool verbose) {
    std::cout << "Configuring Comparative Simulator..." << std::endl;
    
    this->gameManagersFolder = gameManagersFolder;
    this->gameMapFile = gameMapFile;
    this->algorithm1File = algorithm1File;
    this->algorithm2File = algorithm2File;
    this->numThreads = numThreads;
    this->verbose = verbose;
    
    // Load algorithm libraries first
    loadAlgorithmLibraries();
    
    // Load all GameManager libraries from the folder
    loadGameManagerLibraries();
    
    // Setup thread pool
    setupThreadPool();
    
    // Validate everything is ready
    validateConfiguration();
    
    std::cout << "Comparative configuration complete!" << std::endl;
}

void ComparativeSimulator::loadGameManagerLibraries() {
    std::cout << "Loading GameManager libraries from folder: " << gameManagersFolder << std::endl;
    
    // Find all .so files in the gameManagersFolder
    std::vector<std::string> soFiles;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(gameManagersFolder)) {
            if (entry.path().extension() == ".so") {
                soFiles.push_back(entry.path().string());
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to read GameManager folder '" + gameManagersFolder + "': " + e.what());
    }
    
    if (soFiles.empty()) {
        throw std::runtime_error("No .so files found in GameManager folder: " + gameManagersFolder);
    }
    
    gameManagerFactories.clear();
    gameManagerNames.clear();
    
    auto& gmRegistry = getGameManagerFactoryRegistry();
    
    for (const auto& soFile : soFiles) {

        
        // Clear registry before loading each GameManager
        gmRegistry.clear();
        
        try {
            // Load this specific library
            loader.loadLibrary(soFile);
            
            // Capture the factory that was just registered
            if (gmRegistry.size() == 1) {
                gameManagerFactories.push_back(gmRegistry[0]);
                
                // Generate name from filename
                std::filesystem::path path(soFile);
                std::string name = path.stem().string();
                gameManagerNames.push_back(name);
                

            } else {
                throw std::runtime_error("Expected exactly 1 GameManager factory after loading " + soFile + 
                                       ", got " + std::to_string(gmRegistry.size()));
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to load GameManager library " << soFile << ": " << e.what() << std::endl;
            throw;
        }
    }
    
    gmRegistry.clear();
    
    std::cout << "Loaded " << gameManagerFactories.size() << " GameManager(s) with isolated factories" << std::endl;
    

}

void ComparativeSimulator::loadAlgorithmLibraries() {
    std::cout << "Loading Algorithm libraries..." << std::endl;
    
    // Load both algorithm files
    std::vector<std::string> algorithmFiles = {algorithm1File, algorithm2File};
    loader.loadAlgorithmLibraries(algorithmFiles);
    
    auto playerFactories = loader.getPlayerFactories();
    auto algorithmFactories = loader.getTankAlgorithmFactories();
    
    if (playerFactories.empty()) {
        throw std::runtime_error("No Player factories loaded from algorithm files");
    }
    if (algorithmFactories.size() < 2) {
        throw std::runtime_error("Need at least 2 TankAlgorithm factories (for algorithm1 and algorithm2)");
    }
    
    // Use the first player factory for both players
    playerFactory = playerFactories[0];
    
    // Use first two algorithm factories as algorithm1 and algorithm2
    algorithm1Factory = algorithmFactories[0];
    algorithm2Factory = algorithmFactories[1];
    
    std::cout << "Loaded algorithm libraries successfully" << std::endl;
}

void ComparativeSimulator::setupThreadPool() {
    size_t minThreadsNeeded = gameManagerFactories.size();
    size_t actualThreads = std::max(numThreads, minThreadsNeeded);
    
    if (numThreads < minThreadsNeeded) {
        std::cout << "WARNING: Requested " << numThreads << " threads, but need " << minThreadsNeeded 
                  << " threads for " << minThreadsNeeded << " GameManagers." << std::endl;
        std::cout << "         Using " << actualThreads << " threads for stability." << std::endl;
    }
    
    size_t workerThreads = (actualThreads >= 2) ? actualThreads : 0;
    std::cout << "Creating ThreadPool with " << workerThreads << " worker threads" << std::endl;
    threadPool = std::make_unique<ThreadPool>(workerThreads);
}

void ComparativeSimulator::validateConfiguration() const {
    if (gameManagerFactories.empty()) {
        throw std::runtime_error("No GameManager factories loaded");
    }
    
    if (!playerFactory) {
        throw std::runtime_error("No Player factory loaded");
    }
    
    if (!algorithm1Factory || !algorithm2Factory) {
        throw std::runtime_error("Algorithm factories not properly loaded");
    }
    
    if (!threadPool) {
        throw std::runtime_error("ThreadPool not initialized");
    }
    
    // Check if game map file exists
    if (!std::filesystem::exists(gameMapFile)) {
        throw std::runtime_error("Game map file not found: " + gameMapFile);
    }
}

void ComparativeSimulator::runComparative() {
    std::cout << "\n=== Starting Comparative Simulation ===" << std::endl;
    std::cout << "Game Map: " << gameMapFile << std::endl;
    std::cout << "Algorithm 1: " << algorithm1File << std::endl;
    std::cout << "Algorithm 2: " << algorithm2File << std::endl;
    std::cout << "GameManagers: " << gameManagerFactories.size() << std::endl;
    std::cout << "Threads: " << numThreads << std::endl;
    
    auto startTime = std::chrono::steady_clock::now();
    
    // Load the single map
    MapData map;
    if (!map.loadFromFile(gameMapFile, verbose)) {
        throw std::runtime_error("Failed to load game map: " + gameMapFile);
    }
    
    // Queue all GameManager executions
    auto futures = queueAllGameManagers();
    
    std::cout << "Queued " << futures.size() << " GameManager executions" << std::endl;
    
    // Collect results and group by identical outcomes
    resultGroups.clear();
    for (auto& future : futures) {
        try {
            ComparativeExecution execution = future.get();
            
            if (execution.success) {
                resultGroups[execution.result].push_back(execution.gameManagerName);
                std::cout << "✓ " << execution.gameManagerName << " completed" << std::endl;
            } else {
                std::cout << "✗ " << execution.gameManagerName << " - ERROR: " << execution.errorMessage << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error collecting GameManager result: " << e.what() << std::endl;
        }
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "\n=== Comparative Simulation Complete ===" << std::endl;
    std::cout << "Total execution time: " << totalTime.count() << "ms" << std::endl;
    std::cout << "Found " << resultGroups.size() << " unique result groups" << std::endl;
    
    // Generate timestamped output file
    std::string outputFile = generateTimestampedFilename();
    writeResultsToFile(outputFile);
    
    std::cout << "Results written to: " << outputFile << std::endl;
}

std::vector<std::future<ComparativeExecution>> ComparativeSimulator::queueAllGameManagers() {
    std::vector<std::future<ComparativeExecution>> futures;
    
    // Load the single map for all executions
    MapData map;
    if (!map.loadFromFile(gameMapFile, verbose)) {
        throw std::runtime_error("Failed to load game map: " + gameMapFile);
    }
    
    size_t totalGameManagers = gameManagerFactories.size();
    
    size_t actualThreads = threadPool->size();
    if (actualThreads == 0) {
        std::cout << "Using single-threaded execution" << std::endl;
    } else {
        std::cout << "Using multi-threaded execution with " << actualThreads << " worker threads" << std::endl;
    }
    
    //Queue all GameManagers to worker threads (one per GameManager)
    for (size_t gmIdx = 0; gmIdx < totalGameManagers; ++gmIdx) {
        auto future = threadPool->enqueue([=, this]() -> ComparativeExecution {
            std::cout << "Thread ID: " << std::this_thread::get_id() << " processing GameManager " << gmIdx 
                      << " (" << gameManagerNames[gmIdx] << ")" << std::endl;
            return runSingleGame(gmIdx, map);
        });
        futures.push_back(std::move(future));
    }
    
    return futures;
}



ComparativeExecution ComparativeSimulator::runSingleGame(size_t gmIdx, const MapData& map) {
    ComparativeExecution execution;
    execution.gameManagerName = gameManagerNames.empty() ? 
        ("GameManager" + std::to_string(gmIdx)) : gameManagerNames[gmIdx];
    
    try {
        auto gameManager = gameManagerFactories[gmIdx](verbose);
        auto satelliteView = GameRunner::createSatelliteView(map);
        auto player1 = playerFactory(1, map.width, map.height, map.maxSteps, map.numShells);
        auto player2 = playerFactory(2, map.width, map.height, map.maxSteps, map.numShells);
        
        GameResult result = gameManager->run(
            map.width, map.height,
            *satelliteView,
            std::filesystem::path(gameMapFile).filename().string(),
            map.maxSteps, map.numShells,
            *player1, "Player1", *player2, "Player2",
            algorithm1Factory, algorithm2Factory
        );
        
        // Convert to ComparativeGameResult
        execution.result.winner = result.winner;
        execution.result.reason = result.reason;
        execution.result.rounds = result.rounds;
        execution.result.remaining_tanks = result.remaining_tanks;
        execution.result.finalMapState = serializeMapState(result.gameState.get());
        execution.finalMapState = execution.result.finalMapState;
        execution.success = true;
        
    } catch (const std::exception& e) {
        execution.success = false;
        execution.errorMessage = e.what();
    }
    
    return execution;
}

std::string ComparativeSimulator::generateTimestampedFilename() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    ss << "_" << std::setfill('0') << std::setw(3) << ms.count();
    
    return gameManagersFolder + "/comparative_results_" + ss.str() + ".txt";
}

void ComparativeSimulator::writeResultsToFile(const std::string& outputFile) const {
    std::ofstream file;
    std::ostream* out = &std::cout;
    bool fileOpened = false;
    
    // Try to open the file
    try {
        file.open(outputFile);
        if (file.is_open()) {
            out = &file;
            fileOpened = true;
        } else {
            throw std::runtime_error("Cannot create file");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: Cannot create output file '" << outputFile << "': " << e.what() << std::endl;
        std::cerr << "Printing results to screen instead:" << std::endl;
        out = &std::cout;
    }
    
    // Write the required format
    *out << "game_map=" << std::filesystem::path(gameMapFile).filename().string() << std::endl;
    *out << "algorithm1=" << std::filesystem::path(algorithm1File).filename().string() << std::endl;
    *out << "algorithm2=" << std::filesystem::path(algorithm2File).filename().string() << std::endl;
    *out << std::endl;
    
    // Write each result group
    for (const auto& [result, gameManagerNames] : resultGroups) {
        for (size_t i = 0; i < gameManagerNames.size(); ++i) {
            if (i > 0) *out << ",";
            *out << gameManagerNames[i];
        }
        *out << std::endl;
        
        *out << getGameResultMessage(result) << std::endl;
        
        *out << result.rounds << std::endl;
        
        *out << result.finalMapState << std::endl;
        
        if (&result != &resultGroups.rbegin()->first) {
            *out << std::endl;
        }
    }
    
    if (fileOpened) {
        file.close();
    }
}

std::string ComparativeSimulator::serializeMapState(const SatelliteView* satelliteView) const {
    if (!satelliteView) {
        return "Error: No final game state available";
    }
    
    const MapSatelliteView* mapView = dynamic_cast<const MapSatelliteView*>(satelliteView);
    if (!mapView) {
        return "Error: Cannot access map dimensions";
    }
    
    std::stringstream ss;
    size_t width = mapView->getWidth();
    size_t height = mapView->getHeight();
    
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            ss << satelliteView->getObjectAt(x, y);
        }
        if (y < height - 1) {
            ss << std::endl;
        }
    }
    
    return ss.str();
}

std::string ComparativeSimulator::getGameResultMessage(const ComparativeGameResult& result) const {
    std::stringstream ss;
    
    if (result.winner == 0) {
        ss << "Tie";
    } else {
        ss << "Player " << result.winner << " won";
    }
    
    ss << " - ";
    
    switch (result.reason) {
        case GameResult::ALL_TANKS_DEAD:
            ss << "all tanks of the opponent are dead";
            break;
        case GameResult::MAX_STEPS:
            ss << "maximum steps reached";
            break;
        case GameResult::ZERO_SHELLS:
            ss << "all shells exhausted";
            break;
        default:
            ss << "unknown reason";
            break;
    }
    
    return ss.str();
}