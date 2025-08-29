#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <dlfcn.h>
#include <algorithm>
#include <functional>

// Include our actual classes
#include "GameRunner.h"
#include "Registry.h"

namespace fs = std::filesystem;

// Command line argument structure
struct ComparativeArgs {
    std::string gameMap;
    std::string gameManagersFolder;
    std::string algorithm1;
    std::string algorithm2;
    int numThreads = 1;
    bool verbose = false;
    
    bool isValid() const {
        return !gameMap.empty() && !gameManagersFolder.empty() && 
               !algorithm1.empty() && !algorithm2.empty();
    }
};

// Thread pool for parallel execution
class ComparativeThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

public:
    ComparativeThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    ~ComparativeThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers) {
            worker.join();
        }
    }
};

// Game result structure for comparative mode
struct ComparativeGameResult {
    std::string gameManagerName;
    bool success;
    std::string errorMessage;
    
    // Game result fields (extracted from GameExecution)
    int winner;
    GameResult::Reason reason;
    size_t rounds;
    
    // Final game state
    std::string finalGameState;
    
    // Constructor for creating from individual fields
    ComparativeGameResult(std::string name, bool s, std::string error, int w, GameResult::Reason r, size_t rnds, std::string finalState = "")
        : gameManagerName(std::move(name)), success(s), errorMessage(std::move(error)), winner(w), reason(r), rounds(rnds), finalGameState(std::move(finalState)) {}
    
    // Default constructor
    ComparativeGameResult() = default;
    
    // Copy operations are fine now
    ComparativeGameResult(const ComparativeGameResult& other) = default;
    ComparativeGameResult& operator=(const ComparativeGameResult& other) = default;
};

// Parse command line arguments in key=value format
ComparativeArgs parseArgs(int argc, char* argv[]) {
    ComparativeArgs args;
    std::vector<std::string> unsupportedArgs;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        // Handle -verbose flag
        if (arg == "-verbose") {
            args.verbose = true;
            continue;
        }
        
        // Parse key=value format
        size_t equalPos = arg.find('=');
        if (equalPos == std::string::npos) {
            unsupportedArgs.push_back(arg);
            continue;
        }
        
        std::string key = arg.substr(0, equalPos);
        std::string value = arg.substr(equalPos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (key == "game_map") {
            args.gameMap = value;
        } else if (key == "game_managers_folder") {
            args.gameManagersFolder = value;
        } else if (key == "algorithm1") {
            args.algorithm1 = value;
        } else if (key == "algorithm2") {
            args.algorithm2 = value;
        } else if (key == "num_threads") {
            try {
                args.numThreads = std::stoi(value);
                if (args.numThreads < 1) {
                    std::cerr << "Error: num_threads must be >= 1\n";
                    exit(1);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid num_threads value: " << value << "\n";
                exit(1);
            }
        } else {
            unsupportedArgs.push_back(arg);
        }
    }
    
    // Check for unsupported arguments
    if (!unsupportedArgs.empty()) {
        std::cerr << "Error: Unsupported command line arguments:\n";
        for (const auto& arg : unsupportedArgs) {
            std::cerr << "  " << arg << "\n";
        }
        std::cerr << "\nUsage: " << argv[0] << " -comparative game_map=<file> game_managers_folder=<dir> algorithm1=<file> algorithm2=<file> [num_threads=<n>] [-verbose]\n";
        exit(1);
    }
    
    // Check for missing required arguments
    std::vector<std::string> missingArgs;
    if (args.gameMap.empty()) missingArgs.push_back("game_map");
    if (args.gameManagersFolder.empty()) missingArgs.push_back("game_managers_folder");
    if (args.algorithm1.empty()) missingArgs.push_back("algorithm1");
    if (args.algorithm2.empty()) missingArgs.push_back("algorithm2");
    
    if (!missingArgs.empty()) {
        std::cerr << "Error: Missing required command line arguments:\n";
        for (const auto& arg : missingArgs) {
            std::cerr << "  " << arg << "\n";
        }
        std::cerr << "\nUsage: " << argv[0] << " -comparative game_map=<file> game_managers_folder=<dir> algorithm1=<file> algorithm2=<file> [num_threads=<n>] [-verbose]\n";
        exit(1);
    }
    
    return args;
}

// Validate file and folder existence
void validatePaths(const ComparativeArgs& args) {
    // Check game map file
    if (!fs::exists(args.gameMap)) {
        std::cerr << "Error: Game map file does not exist: " << args.gameMap << "\n";
        exit(1);
    }
    
    // Check game managers folder
    if (!fs::exists(args.gameManagersFolder) || !fs::is_directory(args.gameManagersFolder)) {
        std::cerr << "Error: Game managers folder does not exist or is not a directory: " << args.gameManagersFolder << "\n";
        exit(1);
    }
    
    // Check algorithm files
    if (!fs::exists(args.algorithm1)) {
        std::cerr << "Error: Algorithm1 file does not exist: " << args.algorithm1 << "\n";
        exit(1);
    }
    
    if (!fs::exists(args.algorithm2)) {
        std::cerr << "Error: Algorithm2 file does not exist: " << args.algorithm2 << "\n";
        exit(1);
    }
    
    // Check if game managers folder has any .so files
    bool hasSoFiles = false;
    for (const auto& entry : fs::directory_iterator(args.gameManagersFolder)) {
        if (entry.is_regular_file() && entry.path().extension() == ".so") {
            hasSoFiles = true;
            break;
        }
    }
    
    if (!hasSoFiles) {
        std::cerr << "Error: Game managers folder contains no .so files: " << args.gameManagersFolder << "\n";
        exit(1);
    }
}

// Get timestamp string for output filename
std::string getTimestampString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    ss << "_" << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// Load and run a single game
ComparativeGameResult runSingleGame(
    const std::string& gameManagerPath,
    const MapData& map,
    bool verbose,
    const std::vector<PlayerFactory>& playerFactories,
    const std::vector<TankAlgorithmFactory>& tankAlgorithmFactories
) {
    ComparativeGameResult result;
    result.gameManagerName = fs::path(gameManagerPath).stem().string();
    result.success = false;
    
    std::cout << "DEBUG: Starting runSingleGame for GameManager: " << result.gameManagerName << std::endl;
    std::cout << "DEBUG: GameManager path: " << gameManagerPath << std::endl;
    
    try {
        // Clear any existing GameManager registrations before loading new library
        std::cout << "DEBUG: Clearing existing GameManager registrations..." << std::endl;
        {
            std::lock_guard<std::mutex> lock(registryMutex);
            gameManagerFactories.clear();
            gameManagerNames.clear();
        }
        
        // Load GameManager library
        std::cout << "DEBUG: Loading GameManager library..." << std::endl;
        void* gameManagerHandle = dlopen(gameManagerPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (!gameManagerHandle) {
            result.errorMessage = "Failed to load GameManager library: " + std::string(dlerror());
            std::cout << "DEBUG: FAILED to load GameManager: " << result.errorMessage << std::endl;
            return result;
        }
        std::cout << "DEBUG: Successfully loaded GameManager library" << std::endl;
        
        // Wait for static initialization to complete
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Check what GameManager was registered
        std::cout << "DEBUG: Checking GameManager registrations..." << std::endl;
        std::lock_guard<std::mutex> lock(registryMutex);
        std::cout << "DEBUG: Registered GameManagers: " << gameManagerFactories.size() << std::endl;
        
        if (gameManagerFactories.empty()) {
            dlclose(gameManagerHandle);
            result.errorMessage = "No GameManager factories registered after loading library";
            std::cout << "DEBUG: NO GameManager factories registered!" << std::endl;
            return result;
        }
        
        // Use the registered GameManager factory and the pre-loaded algorithm factories
        // Make copies to avoid holding references to registry after library is closed
        GameManagerFactory gameManagerFactory = gameManagerFactories[0];
        PlayerFactory playerFactory = playerFactories[0];
        TankAlgorithmFactory tankAlgorithmFactory = tankAlgorithmFactories[0];
        
        std::cout << "DEBUG: Using registered GameManager factory and pre-loaded algorithm factories" << std::endl;
        
        // Run the game
        std::cout << "DEBUG: Calling GameRunner::runSingleGame..." << std::endl;
        GameExecution execution = GameRunner::runSingleGame(
            gameManagerFactory,
            result.gameManagerName,
            playerFactory,
            tankAlgorithmFactory,
            "TankAlgorithm_318772340_206580102",  // First algorithm name
            "TankAlgorithm_318772340_206580102",  // Second algorithm name (same in comparative mode)
            map,
            verbose
        );
        
        std::cout << "DEBUG: GameRunner::runSingleGame completed successfully" << std::endl;
        std::cout << "DEBUG: Game result - winner: " << execution.result.winner << ", reason: " << static_cast<int>(execution.result.reason) << ", rounds: " << execution.result.rounds << std::endl;
        
        result.success = true;
        result.winner = execution.result.winner;
        result.reason = execution.result.reason;
        result.rounds = execution.result.rounds;
        
        // Extract final game state from the game execution result
        std::ostringstream finalStateStream;
        size_t height = map.height;
        size_t width = map.width;
        
        // Try to use the actual final game state if available, otherwise use original map
        if (execution.result.gameState) {
            for (size_t row = 0; row < height; ++row) {
                for (size_t col = 0; col < width; ++col) {
                    char cell = execution.result.gameState->getObjectAt(col, row);
                    finalStateStream << cell;
                }
                if (row < height - 1) {
                    finalStateStream << "\n";
                }
            }
        } else {
            // Fallback to original map data if gameState is not available
            for (size_t row = 0; row < height; ++row) {
                for (size_t col = 0; col < width; ++col) {
                    char cell = map.data[row][col];
                    finalStateStream << cell;
                }
                if (row < height - 1) {
                    finalStateStream << "\n";
                }
            }
        }
        result.finalGameState = finalStateStream.str();
        
        std::cout << "DEBUG: Game completed successfully for " << result.gameManagerName << std::endl;
        std::cout << "DEBUG: About to return from runSingleGame..." << std::endl;
        std::cout << "DEBUG: Final result values - success: " << result.success << ", winner: " << result.winner << ", rounds: " << result.rounds << std::endl;
        
        // Don't close the library here - we'll close all libraries at the end
        // This prevents segmentation faults and endless loops
        std::cout << "DEBUG: Keeping library open to avoid crashes..." << std::endl;
        
        return result;
        
    } catch (const std::exception& e) {
        result.errorMessage = "Exception during game execution: " + std::string(e.what());
        std::cout << "DEBUG: EXCEPTION caught in runSingleGame: " << result.errorMessage << std::endl;
    } catch (...) {
        result.errorMessage = "Unknown exception during game execution";
        std::cout << "DEBUG: UNKNOWN EXCEPTION caught in runSingleGame" << std::endl;
    }
    
    std::cout << "DEBUG: runSingleGame finished for " << result.gameManagerName << " with success=" << result.success << std::endl;
    if (!result.success) {
        std::cout << "DEBUG: Error message: " << result.errorMessage << std::endl;
    }
    
    std::cout << "DEBUG: About to return result from runSingleGame..." << std::endl;
    return result;
}

// Group results by identical outcomes
std::vector<std::vector<ComparativeGameResult>> groupResults(const std::vector<ComparativeGameResult>& results) {
    std::vector<std::vector<ComparativeGameResult>> groups;
    std::vector<bool> processed(results.size(), false);
    
    for (size_t i = 0; i < results.size(); ++i) {
        if (processed[i] || !results[i].success) continue;
        
        std::vector<ComparativeGameResult> group;
        group.push_back(results[i]);
        processed[i] = true;
        
        for (size_t j = i + 1; j < results.size(); ++j) {
            if (processed[j] || !results[j].success) continue;
            
            // Compare results
            const auto& result1 = results[i];
            const auto& result2 = results[j];
            
            if (result1.winner == result2.winner &&
                result1.reason == result2.reason &&
                result1.rounds == result2.rounds &&
                result1.finalGameState == result2.finalGameState) {
                group.push_back(results[j]);
                processed[j] = true;
            }
        }
        
        groups.push_back(std::move(group));
    }
    
    return groups;
}

// Write results to output file
void writeResults(
    const std::string& outputPath,
    const ComparativeArgs& args,
    const std::vector<std::vector<ComparativeGameResult>>& groups
) {
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create output file: " << outputPath << "\n";
        std::cerr << "Writing results to screen instead:\n\n";
        
        // Write to screen
        std::cout << "game_map=" << args.gameMap << "\n";
        std::cout << "algorithm1=" << args.algorithm1 << "\n";
        std::cout << "algorithm2=" << args.algorithm2 << "\n\n";
        
        for (const auto& group : groups) {
            // Write comma-separated game manager names
            for (size_t i = 0; i < group.size(); ++i) {
                if (i > 0) std::cout << ",";
                std::cout << group[i].gameManagerName;
            }
            std::cout << "\n";
            
            // Write game result
            const auto& result = group[0];
            std::string winner = (result.winner == 0 ? "Tie" : 
                                 result.winner == 1 ? "Player1" : "Player2");
            std::string reason = (result.reason == GameResult::ALL_TANKS_DEAD ? "All tanks dead" :
                                 result.reason == GameResult::MAX_STEPS ? "Max steps" : "Zero shells");
            
            if (result.winner == 0) {
                std::cout << "Tie in round " << result.rounds << " (" << reason << ")\n";
            } else {
                std::cout << winner << " wins in round " << result.rounds << " (" << reason << ")\n";
            }
            std::cout << result.rounds << "\n";
            
            // Write final game state
            std::cout << result.finalGameState << "\n";
            std::cout << "\n";
        }
        return;
    }
    
    // Write to file
    file << "game_map=" << args.gameMap << "\n";
    file << "algorithm1=" << args.algorithm1 << "\n";
    file << "algorithm2=" << args.algorithm2 << "\n\n";
    
    for (const auto& group : groups) {
        // Write comma-separated game manager names
        for (size_t i = 0; i < group.size(); ++i) {
            if (i > 0) file << ",";
            file << group[i].gameManagerName;
        }
        file << "\n";
        
        // Write game result
        const auto& result = group[0];
        std::string winner = (result.winner == 0 ? "Tie" : 
                             result.winner == 1 ? "Player1" : "Player2");
        std::string reason = (result.reason == GameResult::ALL_TANKS_DEAD ? "All tanks dead" :
                             result.reason == GameResult::MAX_STEPS ? "Max steps" : "Zero shells");
        
        if (result.winner == 0) {
            file << "Tie in round " << result.rounds << " (" << reason << ")\n";
        } else {
            file << winner << " wins in round " << result.rounds << " (" << reason << ")\n";
        }
        file << result.rounds << "\n";
        
        // Write final game state
        file << result.finalGameState << "\n";
        file << "\n";
    }
    
    file.close();
    std::cout << "Results written to: " << outputPath << "\n";
}

int main(int argc, char* argv[]) {
    // Check for comparative mode flag
    if (argc < 2 || std::string(argv[1]) != "-comparative") {
        std::cerr << "Usage: " << argv[0] << " -comparative game_map=<file> game_managers_folder=<dir> algorithm1=<file> algorithm2=<file> [num_threads=<n>] [-verbose]\n";
        return 1;
    }
    
    // Parse arguments (skip the -comparative flag)
    char** args = argv + 1;
    ComparativeArgs config = parseArgs(argc - 1, args);
    
    // Validate paths
    validatePaths(config);
    
    // Load map
    MapData map;
    if (!map.loadFromFile(config.gameMap, config.verbose)) {
        std::cerr << "Error: Failed to load game map: " << config.gameMap << "\n";
        return 1;
    }
    
    // Collect game manager .so files
    std::vector<std::string> gameManagerFiles;
    for (const auto& entry : fs::directory_iterator(config.gameManagersFolder)) {
        if (entry.is_regular_file() && entry.path().extension() == ".so") {
            gameManagerFiles.push_back(entry.path().string());
        }
    }
    
    if (gameManagerFiles.empty()) {
        std::cerr << "Error: No .so files found in game managers folder: " << config.gameManagersFolder << "\n";
        return 1;
    }
    
    std::cout << "Found " << gameManagerFiles.size() << " GameManager files\n";
    std::cout << "Using " << config.numThreads << " thread(s)\n";
    
    // Load algorithm libraries ONCE at the beginning
    std::cout << "DEBUG: Loading algorithm libraries..." << std::endl;
    
    // Clear any existing registrations
    {
        std::lock_guard<std::mutex> lock(registryMutex);
        playerFactories.clear();
        playerNames.clear();
        tankAlgorithmFactories.clear();
        tankAlgorithmNames.clear();
    }
    
    // Load Algorithm1 library
    std::cout << "DEBUG: Loading Algorithm1 library: " << config.algorithm1 << std::endl;
    void* algorithm1Handle = dlopen(config.algorithm1.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!algorithm1Handle) {
        std::cerr << "Error: Failed to load Algorithm1 library: " << dlerror() << "\n";
        return 1;
    }
    
    // Load Algorithm2 library
    std::cout << "DEBUG: Loading Algorithm2 library: " << config.algorithm2 << std::endl;
    void* algorithm2Handle = dlopen(config.algorithm2.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!algorithm2Handle) {
        dlclose(algorithm1Handle);
        std::cerr << "Error: Failed to load Algorithm2 library: " << dlerror() << "\n";
        return 1;
    }
    
    // Wait for static initialization to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Check what algorithms were registered
    std::cout << "DEBUG: Checking algorithm registrations..." << std::endl;
    {
        std::lock_guard<std::mutex> lock(registryMutex);
        std::cout << "DEBUG: Registered Players: " << playerFactories.size() << std::endl;
        std::cout << "DEBUG: Registered TankAlgorithms: " << tankAlgorithmFactories.size() << std::endl;
        
        if (playerFactories.empty() || tankAlgorithmFactories.empty()) {
            dlclose(algorithm2Handle);
            dlclose(algorithm1Handle);
            std::cerr << "Error: No algorithms registered after loading libraries\n";
            return 1;
        }
    }
    
    std::cout << "DEBUG: Successfully loaded algorithm libraries" << std::endl;
    
    // Run games
    std::vector<ComparativeGameResult> results;
    results.reserve(gameManagerFiles.size());
    
    std::cout << "DEBUG: Starting to run " << gameManagerFiles.size() << " games..." << std::endl;
    
    if (config.numThreads == 1) {
        // Single-threaded execution
        std::cout << "DEBUG: Using single-threaded execution" << std::endl;
        for (const auto& gameManagerFile : gameManagerFiles) {
            std::cout << "DEBUG: Processing GameManager: " << fs::path(gameManagerFile).filename() << std::endl;
            
            if (config.verbose) {
                std::cout << "Running game with: " << fs::path(gameManagerFile).filename() << "\n";
            }
            
            auto result = runSingleGame(gameManagerFile, map, config.verbose, playerFactories, tankAlgorithmFactories);
            std::cout << "DEBUG: Got result for " << fs::path(gameManagerFile).filename() << " - success: " << result.success << std::endl;
            
            results.push_back(std::move(result));
            
            if (config.verbose) {
                if (result.success) {
                    std::cout << "  Completed successfully\n";
                } else {
                    std::cout << "  Failed: " << result.errorMessage << "\n";
                }
            }
        }
    } else {
        // Multi-threaded execution
        std::cout << "DEBUG: Using multi-threaded execution with " << config.numThreads << " threads" << std::endl;
        ComparativeThreadPool pool(config.numThreads);
        std::vector<std::future<ComparativeGameResult>> futures;
        
        for (const auto& gameManagerFile : gameManagerFiles) {
            std::cout << "DEBUG: Enqueueing GameManager: " << fs::path(gameManagerFile).filename() << std::endl;
            futures.push_back(pool.enqueue([gameManagerFile, &map, &config]() {
                return runSingleGame(gameManagerFile, map, false, playerFactories, tankAlgorithmFactories);
            }));
        }
        
        // Collect results
        std::cout << "DEBUG: Collecting results from " << futures.size() << " futures..." << std::endl;
        for (size_t i = 0; i < futures.size(); ++i) {
            std::cout << "DEBUG: Getting result from future " << i << std::endl;
            auto result = futures[i].get();
            std::cout << "DEBUG: Future " << i << " result - success: " << result.success << std::endl;
            results.push_back(std::move(result));
        }
    }
    
    std::cout << "DEBUG: All games completed. Total results: " << results.size() << std::endl;
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "DEBUG: Result " << i << " - GameManager: " << results[i].gameManagerName 
                  << ", Success: " << results[i].success;
        if (!results[i].success) {
            std::cout << ", Error: " << results[i].errorMessage;
        }
        std::cout << std::endl;
    }
    
    // Group results by identical outcomes
    auto groups = groupResults(results);
    
    // Generate output filename
    std::string timestamp = getTimestampString();
    std::string outputPath = fs::path(config.gameManagersFolder) / ("comparative_results_" + timestamp + ".txt");
    
    // Write results
    writeResults(outputPath, config, groups);
    
    // Print summary
    std::cout << "\nSummary:\n";
    std::cout << "  Total GameManagers: " << gameManagerFiles.size() << "\n";
    std::cout << "  Successful games: " << std::count_if(results.begin(), results.end(), [](const auto& r) { return r.success; }) << "\n";
    std::cout << "  Failed games: " << std::count_if(results.begin(), results.end(), [](const auto& r) { return !r.success; }) << "\n";
    std::cout << "  Result groups: " << groups.size() << "\n";
    
    // Cleanup algorithm libraries
    dlclose(algorithm2Handle);
    dlclose(algorithm1Handle);
    
    return 0;
} 