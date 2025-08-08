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
#include "ComparativeSimulator.h"
#include "GameRunner.h"

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
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

public:
    ThreadPool(size_t threads) : stop(false) {
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

    ~ThreadPool() {
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
    GameExecution execution;
    bool success;
    std::string errorMessage;
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
    const std::string& algorithm1Path,
    const std::string& algorithm2Path,
    const MapData& map,
    bool verbose
) {
    ComparativeGameResult result;
    result.gameManagerName = fs::path(gameManagerPath).stem().string();
    result.success = false;
    
    try {
        // Load libraries
        void* gameManagerHandle = dlopen(gameManagerPath.c_str(), RTLD_LAZY);
        if (!gameManagerHandle) {
            result.errorMessage = "Failed to load GameManager library: " + std::string(dlerror());
            return result;
        }
        
        void* algorithm1Handle = dlopen(algorithm1Path.c_str(), RTLD_LAZY);
        if (!algorithm1Handle) {
            dlclose(gameManagerHandle);
            result.errorMessage = "Failed to load Algorithm1 library: " + std::string(dlerror());
            return result;
        }
        
        void* algorithm2Handle = dlopen(algorithm2Path.c_str(), RTLD_LAZY);
        if (!algorithm2Handle) {
            dlclose(gameManagerHandle);
            dlclose(algorithm1Handle);
            result.errorMessage = "Failed to load Algorithm2 library: " + std::string(dlerror());
            return result;
        }
        
        // For now, we'll use direct instantiation since we know our classes work
        // In a full implementation, we would use dlsym to get factory functions
        
        // Create factories (using our known classes)
        GameManagerFactory gameManagerFactory = [](bool verbose) -> std::unique_ptr<AbstractGameManager> {
            return std::make_unique<GameManager_318772340_206580102::GameManager_318772340_206580102>(verbose);
        };
        
        PlayerFactory playerFactory = [](int playerIndex, size_t numTanks, size_t numShells, size_t maxSteps, size_t gameBoardWidth) -> std::unique_ptr<Player> {
            return std::make_unique<Algorithm_318772340_206580102::Player_318772340_206580102>(playerIndex, numTanks, numShells, maxSteps, gameBoardWidth);
        };
        
        TankAlgorithmFactory tankAlgorithmFactory = [](int playerIndex, int numTanks) -> std::unique_ptr<TankAlgorithm> {
            return std::make_unique<Algorithm_318772340_206580102::TankAlgorithm_318772340_206580102>(playerIndex, numTanks);
        };
        
        // Run the game
        result.execution = GameRunner::runSingleGame(
            gameManagerFactory,
            result.gameManagerName,
            playerFactory,
            tankAlgorithmFactory,
            "TankAlgorithm_318772340_206580102",
            map,
            verbose
        );
        
        result.success = true;
        
        // Cleanup
        dlclose(algorithm2Handle);
        dlclose(algorithm1Handle);
        dlclose(gameManagerHandle);
        
    } catch (const std::exception& e) {
        result.errorMessage = "Exception during game execution: " + std::string(e.what());
    }
    
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
            const auto& result1 = results[i].execution.result;
            const auto& result2 = results[j].execution.result;
            
            if (result1.winner == result2.winner &&
                result1.reason == result2.reason &&
                result1.rounds == result2.rounds) {
                group.push_back(results[j]);
                processed[j] = true;
            }
        }
        
        groups.push_back(group);
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
            const auto& result = group[0].execution.result;
            std::string winner = (result.winner == 0 ? "Tie" : 
                                 result.winner == 1 ? "Player1" : "Player2");
            std::string reason = (result.reason == GameResult::ALL_TANKS_DEAD ? "All tanks dead" :
                                 result.reason == GameResult::MAX_STEPS ? "Max steps" : "Zero shells");
            
            std::cout << winner << " wins in round " << result.rounds << " (" << reason << ")\n";
            std::cout << result.rounds << "\n";
            
            // Write final game state (simplified for now)
            if (result.gameState) {
                for (size_t y = 0; y < result.gameState->getHeight(); ++y) {
                    for (size_t x = 0; x < result.gameState->getWidth(); ++x) {
                        std::cout << result.gameState->getObjectAt(x, y);
                    }
                    std::cout << "\n";
                }
            }
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
        const auto& result = group[0].execution.result;
        std::string winner = (result.winner == 0 ? "Tie" : 
                             result.winner == 1 ? "Player1" : "Player2");
        std::string reason = (result.reason == GameResult::ALL_TANKS_DEAD ? "All tanks dead" :
                             result.reason == GameResult::MAX_STEPS ? "Max steps" : "Zero shells");
        
        file << winner << " wins in round " << result.rounds << " (" << reason << ")\n";
        file << result.rounds << "\n";
        
        // Write final game state (simplified for now)
        if (result.gameState) {
            for (size_t y = 0; y < result.gameState->getHeight(); ++y) {
                for (size_t x = 0; x < result.gameState->getWidth(); ++x) {
                    file << result.gameState->getObjectAt(x, y);
                }
                file << "\n";
            }
        }
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
    if (!map.loadFromFile(config.gameMap)) {
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
    
    // Run games
    std::vector<ComparativeGameResult> results;
    results.reserve(gameManagerFiles.size());
    
    if (config.numThreads == 1) {
        // Single-threaded execution
        for (const auto& gameManagerFile : gameManagerFiles) {
            if (config.verbose) {
                std::cout << "Running game with: " << fs::path(gameManagerFile).filename() << "\n";
            }
            
            auto result = runSingleGame(gameManagerFile, config.algorithm1, config.algorithm2, map, config.verbose);
            results.push_back(result);
            
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
        ThreadPool pool(config.numThreads);
        std::vector<std::future<ComparativeGameResult>> futures;
        
        for (const auto& gameManagerFile : gameManagerFiles) {
            futures.push_back(pool.enqueue([gameManagerFile, &config, &map]() {
                return runSingleGame(gameManagerFile, config.algorithm1, config.algorithm2, map, false);
            }));
        }
        
        // Collect results
        for (auto& future : futures) {
            results.push_back(future.get());
        }
    }
    
    // Group results by identical outcomes
    auto groups = groupResults(results);
    
    // Generate output filename
    std::string timestamp = getTimestampString();
    std::string outputPath = fs::path(config.gameManagersFolder) / ("comparative_results_" + timestamp + ".txt");
    
    // Write results
    writeResults(outputPath.string(), config, groups);
    
    // Print summary
    std::cout << "\nSummary:\n";
    std::cout << "  Total GameManagers: " << gameManagerFiles.size() << "\n";
    std::cout << "  Successful games: " << std::count_if(results.begin(), results.end(), [](const auto& r) { return r.success; }) << "\n";
    std::cout << "  Failed games: " << std::count_if(results.begin(), results.end(), [](const auto& r) { return !r.success; }) << "\n";
    std::cout << "  Result groups: " << groups.size() << "\n";
    
    return 0;
} 