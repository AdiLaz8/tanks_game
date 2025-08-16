#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
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
#include <signal.h>
#include <sys/time.h>

// Include our actual classes
#include "GameRunner.h"
#include "Registry.h"
#include "../GameManager/GameManager_318772340_206580102.h"
#include "../Algorithm/Player_318772340_206580102.h"
#include "../Algorithm/TankAlgorithm_318772340_206580102.h"

namespace fs = std::filesystem;

// Command line argument structure for competitive mode
struct CompetitiveArgs {
    std::string gameMapsFolder;
    std::string gameManager;      // single .so file
    std::string algorithmsFolder; 
    int numThreads = 1;
    bool verbose = false;
    
    bool isValid() const {
        return !gameMapsFolder.empty() && !gameManager.empty() && 
               !algorithmsFolder.empty();
    }
};

// Game pairing structure for tournament
struct GamePairing {
    std::string mapName;
    MapData mapData;
    std::string algorithm1Path;
    std::string algorithm2Path;
    int algorithm1Index;
    int algorithm2Index;
};

// Algorithm scoring structure
struct AlgorithmScore {
    std::string algorithmName;
    int totalScore = 0;
    int wins = 0;
    int ties = 0;
    int losses = 0;
};

// Competition game result structure
struct CompetitiveGameResult {
    bool success;
    std::string errorMessage;
    int winner; // 0=tie, 1=algorithm1, 2=algorithm2
    GameResult::Reason reason;
    size_t rounds;
    std::string algorithm1Name;
    std::string algorithm2Name;
    std::string mapName;
};

// Thread pool for parallel execution (reuse from comparative)
class CompetitiveThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

public:
    CompetitiveThreadPool(size_t threads) : stop(false) {
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

    ~CompetitiveThreadPool() {
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

// Parse command line arguments in key=value format
CompetitiveArgs parseCompetitiveArgs(int argc, char* argv[]) {
    CompetitiveArgs args;
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
        
        if (key == "game_maps_folder") {
            args.gameMapsFolder = value;
        } else if (key == "game_manager") {
            args.gameManager = value;
        } else if (key == "algorithms_folder") {
            args.algorithmsFolder = value;
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
        std::cerr << "\nUsage: " << argv[0] << " -competition game_maps_folder=<dir> game_manager=<file> algorithms_folder=<dir> [num_threads=<n>] [-verbose]\n";
        exit(1);
    }
    
    // Check for missing required arguments
    std::vector<std::string> missingArgs;
    if (args.gameMapsFolder.empty()) missingArgs.push_back("game_maps_folder");
    if (args.gameManager.empty()) missingArgs.push_back("game_manager");
    if (args.algorithmsFolder.empty()) missingArgs.push_back("algorithms_folder");
    
    if (!missingArgs.empty()) {
        std::cerr << "Error: Missing required command line arguments:\n";
        for (const auto& arg : missingArgs) {
            std::cerr << "  " << arg << "\n";
        }
        std::cerr << "\nUsage: " << argv[0] << " -competition game_maps_folder=<dir> game_manager=<file> algorithms_folder=<dir> [num_threads=<n>] [-verbose]\n";
        exit(1);
    }
    
    return args;
}

// Validate file and folder existence
void validateCompetitivePaths(const CompetitiveArgs& args) {
    // Check game maps folder
    if (!fs::exists(args.gameMapsFolder) || !fs::is_directory(args.gameMapsFolder)) {
        std::cerr << "Error: Game maps folder does not exist or is not a directory: " << args.gameMapsFolder << "\n";
        exit(1);
    }
    
    // Check if game maps folder has any map files
    bool hasMapFiles = false;
    for (const auto& entry : fs::directory_iterator(args.gameMapsFolder)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            hasMapFiles = true;
            break;
        }
    }
    if (!hasMapFiles) {
        std::cerr << "Error: Game maps folder contains no .txt map files: " << args.gameMapsFolder << "\n";
        exit(1);
    }
    
    // Check game manager file
    if (!fs::exists(args.gameManager)) {
        std::cerr << "Error: Game manager file does not exist: " << args.gameManager << "\n";
        exit(1);
    }
    
    // Check algorithms folder
    if (!fs::exists(args.algorithmsFolder) || !fs::is_directory(args.algorithmsFolder)) {
        std::cerr << "Error: Algorithms folder does not exist or is not a directory: " << args.algorithmsFolder << "\n";
        exit(1);
    }
    
    // Check if algorithms folder has at least 2 .so files
    std::vector<std::string> algorithmFiles;
    for (const auto& entry : fs::directory_iterator(args.algorithmsFolder)) {
        if (entry.is_regular_file() && entry.path().extension() == ".so") {
            algorithmFiles.push_back(entry.path().string());
        }
    }
    if (algorithmFiles.size() < 2) {
        std::cerr << "Error: Algorithms folder must contain at least 2 .so files. Found: " << algorithmFiles.size() << "\n";
        exit(1);
    }
}

// Load maps from folder
std::vector<MapData> loadMapsFromFolder(const std::string& folderPath) {
    std::vector<MapData> maps;
    
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            MapData map;
            if (map.loadFromFile(entry.path().string())) {
                maps.push_back(std::move(map));
            } else {
                std::cerr << "Warning: Failed to load map: " << entry.path().string() << "\n";
            }
        }
    }
    
    // Sort maps by name for consistent ordering
    std::sort(maps.begin(), maps.end(), [](const MapData& a, const MapData& b) {
        return a.name < b.name;
    });
    
    return maps;
}

// Get algorithm files from folder
std::vector<std::string> getAlgorithmFiles(const std::string& folderPath) {
    std::vector<std::string> algorithmFiles;
    
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".so") {
            algorithmFiles.push_back(entry.path().string());
        }
    }
    
    // Sort for consistent ordering
    std::sort(algorithmFiles.begin(), algorithmFiles.end());
    
    return algorithmFiles;
}

// Generate competitive pairings according to assignment specification
std::vector<GamePairing> generateCompetitivePairings(
    const std::vector<MapData>& maps,
    const std::vector<std::string>& algorithmFiles
) {
    std::vector<GamePairing> pairings;
    int N = static_cast<int>(algorithmFiles.size());
    
    if (N < 2) {
        std::cerr << "Error: Need at least 2 algorithms for competition\n";
        return pairings;
    }
    
    std::set<std::tuple<int, int, int>> allPairs; // (mapIndex, alg1, alg2) - to track all unique pairs
    
    for (int k = 0; k < static_cast<int>(maps.size()); ++k) {
        for (int i = 0; i < N; ++i) {
            int opponent = (i + 1 + k % (N-1)) % N;
            
            // Create canonical pair (smaller index first)
            int alg1 = std::min(i, opponent);
            int alg2 = std::max(i, opponent);
            
            // Skip self-play (should not happen with the formula, but safety check)
            if (alg1 == alg2) continue;
            
            auto pairTuple = std::make_tuple(k, alg1, alg2);
            
            // Skip if we've already scheduled this exact pairing for this map
            if (allPairs.find(pairTuple) != allPairs.end()) {
                continue;
            }
            allPairs.insert(pairTuple);
            
            GamePairing pairing;
            pairing.mapName = maps[k].name;
            pairing.mapData = maps[k];
            pairing.algorithm1Path = algorithmFiles[alg1];
            pairing.algorithm2Path = algorithmFiles[alg2];
            pairing.algorithm1Index = alg1;
            pairing.algorithm2Index = alg2;
            pairings.push_back(pairing);
        }
    }
    
    return pairings;
}

// Get timestamp string for output filename
std::string getCompetitiveTimestampString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    ss << "_" << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// Global flag for timeout
static volatile bool gameTimeoutOccurred = false;

// Signal handler for timeout
void timeoutHandler(int sig) {
    gameTimeoutOccurred = true;
}

// Run a single competitive game
CompetitiveGameResult runCompetitiveGame(
    const std::string& gameManagerPath,
    const GamePairing& pairing,
    bool verbose
) {
    CompetitiveGameResult result;
    result.success = false;
    result.mapName = pairing.mapName;
    result.algorithm1Name = fs::path(pairing.algorithm1Path).stem().string();
    result.algorithm2Name = fs::path(pairing.algorithm2Path).stem().string();
    
    if (verbose) {
        std::cout << "Running: " << result.algorithm1Name << " vs " << result.algorithm2Name 
                  << " on " << result.mapName << std::endl;
    }
    
    try {
        if (verbose) {
            std::cout << "DEBUG: Starting game execution" << std::endl;
            std::cout << "DEBUG: Algorithm1 path: " << pairing.algorithm1Path << std::endl;
            std::cout << "DEBUG: Algorithm2 path: " << pairing.algorithm2Path << std::endl;
        }
        
        // Clear only algorithm registrations for clean loading
        // DON'T clear GameManager registry as it won't re-register if already loaded
        getPlayerFactoryRegistry().clear();
        getTankAlgorithmFactoryRegistry().clear();
        
        if (verbose) {
            std::cout << "DEBUG: Cleared registries" << std::endl;
        }
        
        // Load GameManager library (may already be loaded)
        void* gameManagerHandle = dlopen(gameManagerPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (!gameManagerHandle) {
            result.errorMessage = "Failed to load GameManager: " + std::string(dlerror());
            if (verbose) std::cout << "DEBUG: GameManager load failed: " << result.errorMessage << std::endl;
            return result;
        }
        
        if (verbose) {
            std::cout << "DEBUG: GameManager loaded successfully" << std::endl;
        }
        
        // Get current registries
        auto& playerFactories = getPlayerFactoryRegistry();
        auto& tankAlgorithmFactories = getTankAlgorithmFactoryRegistry();
        auto& gameManagerFactories = getGameManagerFactoryRegistry();
        
        // If GameManager isn't registered, we have a problem
        if (gameManagerFactories.empty()) {
            result.errorMessage = "GameManager did not register after loading";
            if (verbose) std::cout << "DEBUG: GameManager registration failed: " << result.errorMessage << std::endl;
            return result;
        }
        
        // Load Algorithm1 library first
        if (verbose) {
            std::cout << "DEBUG: Attempting to load Algorithm1: " << pairing.algorithm1Path << std::endl;
        }
        void* alg1Handle = dlopen(pairing.algorithm1Path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (!alg1Handle) {
            dlclose(gameManagerHandle);
            result.errorMessage = "Failed to load Algorithm1: " + std::string(dlerror());
            if (verbose) {
                std::cout << "DEBUG: Algorithm1 load failed: " << result.errorMessage << std::endl;
                std::cout << "DEBUG: File exists check: " << std::filesystem::exists(pairing.algorithm1Path) << std::endl;
            }
            return result;
        }
        
        if (verbose) {
            std::cout << "DEBUG: Algorithm1 loaded successfully" << std::endl;
        }
        
        if (verbose) {
            std::cout << "DEBUG: After loading GameManager and Algorithm1:" << std::endl;
            std::cout << "DEBUG: GameManager factories: " << gameManagerFactories.size() << std::endl;
            std::cout << "DEBUG: Player factories: " << playerFactories.size() << std::endl;
            std::cout << "DEBUG: TankAlgorithm factories: " << tankAlgorithmFactories.size() << std::endl;
        }
        
        if (gameManagerFactories.empty() || playerFactories.empty() || tankAlgorithmFactories.empty()) {
            result.errorMessage = "No factories registered after loading GameManager and Algorithm1";
            if (verbose) {
                std::cout << "DEBUG: Factory registration failed: " << result.errorMessage << std::endl;
                std::cout << "DEBUG: Detailed analysis:" << std::endl;
                std::cout << "DEBUG:   GameManager factories: " << gameManagerFactories.size() << std::endl;
                std::cout << "DEBUG:   Player factories: " << playerFactories.size() << std::endl;
                std::cout << "DEBUG:   TankAlgorithm factories: " << tankAlgorithmFactories.size() << std::endl;
                std::cout << "DEBUG: This suggests Algorithm1 (" << pairing.algorithm1Path << ") failed to register its factories" << std::endl;
            }
            return result;
        }
        
        // Save Algorithm1 factories
        TankAlgorithmFactory player1AlgoFactory = tankAlgorithmFactories[0];
        PlayerFactory player1PlayerFactory = playerFactories[0];
        GameManagerFactory gameManagerFactory = gameManagerFactories[0];
        
        if (verbose) {
            std::cout << "DEBUG: Saved Algorithm1 factories" << std::endl;
        }
        
        // Load Algorithm2 library if different
        TankAlgorithmFactory player2AlgoFactory;
        PlayerFactory player2PlayerFactory;
        if (pairing.algorithm1Path != pairing.algorithm2Path) {
            if (verbose) {
                std::cout << "DEBUG: Loading different Algorithm2" << std::endl;
            }
            
            // Load Algorithm2 without clearing - it will add to the registry
            void* alg2Handle = dlopen(pairing.algorithm2Path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
            if (!alg2Handle) {
                dlclose(alg1Handle);
                dlclose(gameManagerHandle);
                result.errorMessage = "Failed to load Algorithm2: " + std::string(dlerror());
                if (verbose) std::cout << "DEBUG: Algorithm2 load failed: " << result.errorMessage << std::endl;
                return result;
            }
            
            if (verbose) {
                std::cout << "DEBUG: Algorithm2 loaded successfully" << std::endl;
                std::cout << "DEBUG: TankAlgorithm factories after Algorithm2 load: " << tankAlgorithmFactories.size() << std::endl;
            }
            
            // Algorithm2's factories should be the newest ones (last in the vectors)
            if (tankAlgorithmFactories.size() < 2 || playerFactories.size() < 2) {
                dlclose(alg1Handle);
                dlclose(gameManagerHandle);
                result.errorMessage = "Algorithm2 did not register new factories";
                if (verbose) std::cout << "DEBUG: Algorithm2 factory registration failed: " << result.errorMessage << std::endl;
                return result;
            }
            
            player2AlgoFactory = tankAlgorithmFactories.back(); // Use the most recently added factory
            player2PlayerFactory = playerFactories.back(); // Use the most recently added factory
            if (verbose) {
                std::cout << "DEBUG: Saved Algorithm2 factories (TankAlgorithm index " << tankAlgorithmFactories.size()-1 << ", Player index " << playerFactories.size()-1 << ")" << std::endl;
            }
        } else {
            // Same algorithm for both players
            player2AlgoFactory = player1AlgoFactory;
            player2PlayerFactory = player1PlayerFactory;
            if (verbose) {
                std::cout << "DEBUG: Using same algorithm for both players" << std::endl;
            }
        }
        
        // Create GameManager instance
        auto gameManager = gameManagerFactory(verbose);
        
        if (verbose) {
            std::cout << "DEBUG: GameManager instance created" << std::endl;
        }
        
        // Create Player instances with correct factories for each algorithm
        auto player1 = player1PlayerFactory(1, pairing.mapData.width, pairing.mapData.height, pairing.mapData.maxSteps, pairing.mapData.numShells);
        auto player2 = player2PlayerFactory(2, pairing.mapData.width, pairing.mapData.height, pairing.mapData.maxSteps, pairing.mapData.numShells);
        
        if (verbose) {
            std::cout << "DEBUG: Player instances created" << std::endl;
        }
        
        // Create map satellite view (using the same method as GameRunner)
        auto mapView = GameRunner::createSatelliteView(pairing.mapData);
        
        if (verbose) {
            std::cout << "DEBUG: Map view created, starting game..." << std::endl;
        }
        
        // Run the game directly through GameManager with different algorithms for each player
        // Add timeout to prevent hanging games
        GameResult gameResult;
        
        // Set up timeout mechanism
        gameTimeoutOccurred = false;
        struct sigaction oldAction;
        struct sigaction newAction;
        newAction.sa_handler = timeoutHandler;
        sigemptyset(&newAction.sa_mask);
        newAction.sa_flags = 0;
        
        const int MAX_GAME_TIME_SECONDS = 60; // 1 minute timeout per game
        
        if (verbose) {
            std::cout << "DEBUG: Starting game with " << MAX_GAME_TIME_SECONDS << " second timeout..." << std::endl;
        }
        
        // Set up timer
        struct itimerval timer;
        timer.it_value.tv_sec = MAX_GAME_TIME_SECONDS;
        timer.it_value.tv_usec = 0;
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = 0;
        
        // Install signal handler and start timer
        sigaction(SIGALRM, &newAction, &oldAction);
        setitimer(ITIMER_REAL, &timer, nullptr);
        
        try {
            gameResult = gameManager->run(
                pairing.mapData.width, pairing.mapData.height,
                *mapView,
                pairing.mapData.name,
                pairing.mapData.maxSteps, pairing.mapData.numShells,
                *player1, result.algorithm1Name,
                *player2, result.algorithm2Name,
                player1AlgoFactory,
                player2AlgoFactory
            );
            
            if (verbose) {
                std::cout << "DEBUG: Game completed successfully in reasonable time" << std::endl;
            }
            
        } catch (const std::exception& e) {
            result.errorMessage = "Game execution exception: " + std::string(e.what());
            if (verbose) std::cout << "DEBUG: Game execution failed: " << result.errorMessage << std::endl;
        } catch (...) {
            result.errorMessage = "Unknown game execution error";
            if (verbose) std::cout << "DEBUG: Unknown game execution error" << std::endl;
        }
        
        // Cancel timer and restore signal handler
        setitimer(ITIMER_REAL, nullptr, nullptr);
        sigaction(SIGALRM, &oldAction, nullptr);
        
        // Check if timeout occurred
        if (gameTimeoutOccurred) {
            result.errorMessage = "Game execution timed out after " + std::to_string(MAX_GAME_TIME_SECONDS) + " seconds";
            if (verbose) std::cout << "DEBUG: Game timed out" << std::endl;
            result.success = false;
            return result;
        }
        
        if (verbose) {
            std::cout << "DEBUG: Game completed successfully" << std::endl;
            std::cout << "DEBUG: Winner: " << gameResult.winner << std::endl;
            std::cout << "DEBUG: Reason: " << static_cast<int>(gameResult.reason) << " (0=ALL_TANKS_DEAD, 1=MAX_STEPS, 2=ZERO_SHELLS)" << std::endl;
            std::cout << "DEBUG: Rounds: " << gameResult.rounds << std::endl;
            std::cout << "DEBUG: Remaining tanks: [";
            for (size_t i = 0; i < gameResult.remaining_tanks.size(); ++i) {
                std::cout << gameResult.remaining_tanks[i];
                if (i < gameResult.remaining_tanks.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
            
            // Interpret the result
            if (gameResult.winner == 0) {
                std::cout << "DEBUG: RESULT: TIE" << std::endl;
            } else if (gameResult.winner == 1) {
                std::cout << "DEBUG: RESULT: " << result.algorithm1Name << " WINS" << std::endl;
            } else if (gameResult.winner == 2) {
                std::cout << "DEBUG: RESULT: " << result.algorithm2Name << " WINS" << std::endl;
            }
        }
        
        result.success = true;
        result.winner = gameResult.winner;
        result.reason = gameResult.reason;
        result.rounds = gameResult.rounds;
        
        // Don't close libraries to avoid crashes with remaining objects
        
    } catch (const std::exception& e) {
        result.errorMessage = "Exception: " + std::string(e.what());
        if (verbose) std::cout << "DEBUG: Exception caught: " << result.errorMessage << std::endl;
    } catch (...) {
        result.errorMessage = "Unknown exception";
        if (verbose) std::cout << "DEBUG: Unknown exception caught" << std::endl;
    }
    
    // Additional safety check - if we get here without success, mark as failed
    if (!result.success && result.errorMessage.empty()) {
        result.errorMessage = "Game execution failed for unknown reason";
        if (verbose) std::cout << "DEBUG: Game marked as failed due to unknown reason" << std::endl;
    }
    
    return result;
}

// Calculate algorithm scores
std::vector<AlgorithmScore> calculateScores(
    const std::vector<CompetitiveGameResult>& results,
    const std::vector<std::string>& algorithmFiles
) {
    std::vector<AlgorithmScore> scores;
    
    // Initialize scores for all algorithms
    for (const auto& algFile : algorithmFiles) {
        AlgorithmScore score;
        score.algorithmName = fs::path(algFile).stem().string();
        scores.push_back(score);
    }
    
    // Calculate scores from results
    for (const auto& result : results) {
        if (!result.success) continue;
        
        auto alg1It = std::find_if(scores.begin(), scores.end(),
            [&](const AlgorithmScore& s) { return s.algorithmName == result.algorithm1Name; });
        auto alg2It = std::find_if(scores.begin(), scores.end(),
            [&](const AlgorithmScore& s) { return s.algorithmName == result.algorithm2Name; });
            
        if (alg1It == scores.end() || alg2It == scores.end()) continue;
        
        if (result.winner == 0) { // Tie
            alg1It->totalScore += 1;
            alg1It->ties += 1;
            alg2It->totalScore += 1;
            alg2It->ties += 1;
        } else if (result.winner == 1) { // Algorithm1 wins
            alg1It->totalScore += 3;
            alg1It->wins += 1;
            alg2It->losses += 1;
        } else if (result.winner == 2) { // Algorithm2 wins
            alg2It->totalScore += 3;
            alg2It->wins += 1;
            alg1It->losses += 1;
        }
    }
    
    // Sort by total score (descending)
    std::sort(scores.begin(), scores.end(),
        [](const AlgorithmScore& a, const AlgorithmScore& b) {
            return a.totalScore > b.totalScore;
        });
    
    return scores;
}

// Write competitive results to file
void writeCompetitiveResults(
    const std::string& outputPath,
    const CompetitiveArgs& args,
    const std::vector<AlgorithmScore>& scores
) {
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create output file: " << outputPath << "\n";
        std::cerr << "Writing results to screen instead:\n\n";
        
        // Write to screen
        std::cout << "game_maps_folder=" << args.gameMapsFolder << "\n";
        std::cout << "game_manager=" << args.gameManager << "\n\n";
        for (const auto& score : scores) {
            std::cout << score.algorithmName << " " << score.totalScore << "\n";
        }
        return;
    }
    
    // Write to file
    file << "game_maps_folder=" << args.gameMapsFolder << "\n";
    file << "game_manager=" << args.gameManager << "\n\n";
    for (const auto& score : scores) {
        file << score.algorithmName << " " << score.totalScore << "\n";
    }
    
    file.close();
    std::cout << "Competition results written to: " << outputPath << "\n";
}

int runCompetitive(int argc, char* argv[]) {
    // Check for competition mode flag
    if (argc < 2 || std::string(argv[1]) != "-competition") {
        std::cerr << "Usage: " << argv[0] << " -competition game_maps_folder=<dir> game_manager=<file> algorithms_folder=<dir> [num_threads=<n>] [-verbose]\n";
        return 1;
    }
    
    // Parse arguments (skip the -competition flag)
    char** args = argv + 1;
    CompetitiveArgs config = parseCompetitiveArgs(argc - 1, args);
    
    // Validate paths
    validateCompetitivePaths(config);
    
    // Load maps from folder
    std::vector<MapData> maps = loadMapsFromFolder(config.gameMapsFolder);
    if (maps.empty()) {
        std::cerr << "Error: No valid maps found in folder: " << config.gameMapsFolder << "\n";
        return 1;
    }
    
    // Load algorithm files from folder
    std::vector<std::string> algorithmFiles = getAlgorithmFiles(config.algorithmsFolder);
    if (algorithmFiles.size() < 2) {
        std::cerr << "Error: Need at least 2 algorithms for competition. Found: " << algorithmFiles.size() << "\n";
        return 1;
    }
    
    // Generate competitive pairings
    std::vector<GamePairing> pairings = generateCompetitivePairings(maps, algorithmFiles);
    
    std::cout << "Competition setup:\n";
    std::cout << "  Maps: " << maps.size() << "\n";
    std::cout << "  Algorithms: " << algorithmFiles.size() << "\n";
    std::cout << "  Total games: " << pairings.size() << "\n";
    std::cout << "  Threads: " << config.numThreads << "\n\n";
    
    std::cout << "Running competitive mode..." << std::endl;
    
    // Run all games
    std::vector<CompetitiveGameResult> results;
    results.reserve(pairings.size());
    
    if (config.numThreads == 1) {
        // Single-threaded execution
        for (size_t i = 0; i < pairings.size(); ++i) {
            const auto& pairing = pairings[i];
            
            if (config.verbose) {
                std::cout << "Game " << (i+1) << "/" << pairings.size() << ": ";
            }
            
            auto result = runCompetitiveGame(config.gameManager, pairing, config.verbose);
            results.push_back(std::move(result));
            
            if (!config.verbose) {
                if (result.success) {
                    std::cout << "✓ Game " << (i+1) << "/" << pairings.size() << " completed" << std::endl;
                } else {
                    std::cout << "✗ Game " << (i+1) << "/" << pairings.size() << " failed: " << result.errorMessage << std::endl;
                }
            }
        }
    } else {
        // Multi-threaded execution
        CompetitiveThreadPool pool(config.numThreads);
        std::vector<std::future<CompetitiveGameResult>> futures;
        
        for (const auto& pairing : pairings) {
            futures.push_back(pool.enqueue([&config, pairing]() {
                return runCompetitiveGame(config.gameManager, pairing, false);
            }));
        }
        
        // Collect results
        for (size_t i = 0; i < futures.size(); ++i) {
            auto result = futures[i].get();
            results.push_back(std::move(result));
            
            if (result.success) {
                std::cout << "✓ Game " << (i+1) << "/" << futures.size() << " completed" << std::endl;
            } else {
                std::cout << "✗ Game " << (i+1) << "/" << futures.size() << " failed: " << result.errorMessage << std::endl;
            }
        }
    }
    
    // Calculate scores
    auto scores = calculateScores(results, algorithmFiles);
    
    // Generate output filename
    std::string timestamp = getCompetitiveTimestampString();
    std::string outputPath = fs::path(config.algorithmsFolder) / ("competition_" + timestamp + ".txt");
    
    // Write results
    writeCompetitiveResults(outputPath, config, scores);
    
    // Print summary
    std::cout << "\nCompetition Results:\n";
    for (const auto& score : scores) {
        std::cout << "  " << score.algorithmName << ": " << score.totalScore 
                  << " points (W:" << score.wins << " T:" << score.ties << " L:" << score.losses << ")\n";
    }
    
    // Print game statistics
    int successfulGames = std::count_if(results.begin(), results.end(), [](const auto& r) { return r.success; });
    int failedGames = results.size() - successfulGames;
    
    std::cout << "\nGame Statistics:\n";
    std::cout << "  Total games: " << results.size() << "\n";
    std::cout << "  Successful games: " << successfulGames << "\n";
    std::cout << "  Failed games: " << failedGames << "\n";
    
    return 0;
}
