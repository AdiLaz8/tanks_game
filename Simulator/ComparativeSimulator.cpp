#include "ComparativeSimulator.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <filesystem>

// ComparativeGameResult comparison operators for grouping
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
    
    // Load the GameManager libraries
    loader.loadGameManagerLibraries(soFiles);
    gameManagerFactories = loader.getGameManagerFactories();
    gameManagerNames = loader.getGameManagerFactoryNames();
    
    std::cout << "Loaded " << gameManagerFactories.size() << " GameManager(s)" << std::endl;
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
    
    // Use the first player factory for both players (they differentiate by player index)
    playerFactory = playerFactories[0];
    
    // Use first two algorithm factories as algorithm1 and algorithm2
    algorithm1Factory = algorithmFactories[0];
    algorithm2Factory = algorithmFactories[1];
    
    std::cout << "Loaded algorithm libraries successfully" << std::endl;
}

void ComparativeSimulator::setupThreadPool() {
    // For numThreads >= 2, create numThreads worker threads
    // The main thread will also participate in the work
    size_t workerThreads = (numThreads >= 2) ? numThreads : 0;
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
    
    if (numThreads == 1) {
        // Single-threaded: queue all games for the single worker thread
        for (size_t gmIdx = 0; gmIdx < totalGameManagers; ++gmIdx) {
            auto future = threadPool->enqueue([=, this]() -> ComparativeExecution {
                return runSingleGame(gmIdx, map);
            });
            futures.push_back(std::move(future));
        }
    } else {
        // Multi-threaded: distribute work between worker threads and main thread
        size_t workerThreads = threadPool->size();
        size_t gamesForWorkers = std::min(workerThreads, totalGameManagers);
        
        // Queue games for worker threads
        for (size_t gmIdx = 0; gmIdx < gamesForWorkers; ++gmIdx) {
            auto future = threadPool->enqueue([=, this]() -> ComparativeExecution {
                return runSingleGame(gmIdx, map);
            });
            futures.push_back(std::move(future));
        }
        
        // Main thread runs remaining games
        for (size_t gmIdx = gamesForWorkers; gmIdx < totalGameManagers; ++gmIdx) {
            // Create a packaged_task to wrap the game execution
            auto task = std::packaged_task<ComparativeExecution()>([=, this]() -> ComparativeExecution {
                return runSingleGame(gmIdx, map);
            });
            
            // Get the future from the packaged_task
            futures.push_back(task.get_future());
            
            // Execute the task immediately in the main thread
            task();
        }
    }
    
    return futures;
}



ComparativeExecution ComparativeSimulator::runSingleGame(size_t gmIdx, const MapData& map) {
    ComparativeExecution execution;
    execution.gameManagerName = gameManagerNames.empty() ? 
        ("GameManager" + std::to_string(gmIdx)) : gameManagerNames[gmIdx];
    
    try {
        // Create GameManager
        auto gameManager = gameManagerFactories[gmIdx](verbose);
        
        // Create satellite view from map
        auto satelliteView = GameRunner::createSatelliteView(map);
        
        // Create players with the same factory but different indices
        auto player1 = playerFactory(1, map.width, map.height, map.maxSteps, map.numShells);
        auto player2 = playerFactory(2, map.width, map.height, map.maxSteps, map.numShells);
        
        // Create algorithms with different factories
        auto algorithm1 = algorithm1Factory(1, 0);
        auto algorithm2 = algorithm2Factory(2, 0);
        
        // Run the game
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
        // Line 5: Comma-separated list of GameManager names with identical results
        for (size_t i = 0; i < gameManagerNames.size(); ++i) {
            if (i > 0) *out << ",";
            *out << gameManagerNames[i];
        }
        *out << std::endl;
        
        // Line 6: Game result message
        *out << getGameResultMessage(result) << std::endl;
        
        // Line 7: Round number
        *out << result.rounds << std::endl;
        
        // Line 8+: Final map state
        *out << result.finalMapState << std::endl;
        
        // Empty line before next group (if any)
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
    
    // Try to cast to MapSatelliteView to get dimensions
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