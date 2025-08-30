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
#include <numeric>

// Include our actual classes
#include "GameRunner.h"
#include "Registry.h"

namespace fs = std::filesystem;



// Command line argument structure for competitive mode
struct CompetitiveArgs {
    std::string gameMapsFolder;
    std::string gameManager;
    std::string algorithmsFolder;
    int numThreads = 1;
    bool verbose = false;
    
    bool isValid() const {
        return !gameMapsFolder.empty() && !gameManager.empty() && !algorithmsFolder.empty();
    }
};

// Thread pool for parallel execution
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
            workers.emplace_back([this, i] {
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

// Game result structure for competitive mode
struct CompetitiveGameResult {
    std::string mapName;
    std::string algorithm1;
    std::string algorithm2;
    bool success;
    int winner; // 0 = tie, 1 = algorithm1 wins, 2 = algorithm2 wins
    std::string errorMessage;
    size_t rounds;
};

// Algorithm score structure
struct AlgorithmScore {
    std::string name;
    int totalScore;
    int wins;
    int ties;
    int losses;
    
    AlgorithmScore() : name(""), totalScore(0), wins(0), ties(0), losses(0) {}
    AlgorithmScore(const std::string& n) : name(n), totalScore(0), wins(0), ties(0), losses(0) {}
};

// Parse command line arguments
CompetitiveArgs parseArgs(int argc, char* argv[]) {
    CompetitiveArgs args;
    
    for (int i = 0; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg.find("game_maps_folder=") == 0) {
            args.gameMapsFolder = arg.substr(17);
        } else if (arg.find("game_manager=") == 0) {
            args.gameManager = arg.substr(13);
        } else if (arg.find("algorithms_folder=") == 0) {
            args.algorithmsFolder = arg.substr(18);
        } else if (arg.find("num_threads=") == 0) {
            args.numThreads = std::stoi(arg.substr(12));
        } else if (arg == "-verbose") {
            args.verbose = true;
        }
    }
    
    return args;
}

// Forward declarations
std::vector<std::pair<int, int>> calculatePairings(int mapIndex, int numAlgorithms);
void writeCompetitionResults(const std::string& outputPath, 
                           const CompetitiveArgs& config,
                           const std::vector<AlgorithmScore>& algorithmScores);

// Validate paths
void validatePaths(const CompetitiveArgs& config) {
    if (!fs::exists(config.gameMapsFolder)) {
        throw std::runtime_error("Game maps folder does not exist: " + config.gameMapsFolder);
    }
    if (!fs::exists(config.gameManager)) {
        throw std::runtime_error("Game manager file does not exist: " + config.gameManager);
    }
    if (!fs::exists(config.algorithmsFolder)) {
        throw std::runtime_error("Algorithms folder does not exist: " + config.algorithmsFolder);
    }
}

// Get timestamp string for output filename
std::string getTimestampString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    ss << "_" << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// Run a single competitive game between two algorithms
CompetitiveGameResult runCompetitiveGame(
    const std::string& mapFile,
    const std::string& /*gameManagerFile*/,
    const std::string& algorithm1File,
    const std::string& algorithm2File,
    int algorithm1Index __attribute__((unused)),
    int algorithm2Index __attribute__((unused)),
    bool verbose) {
    
    
    CompetitiveGameResult result;
    result.mapName = fs::path(mapFile).filename().string();
    result.algorithm1 = fs::path(algorithm1File).filename().string();
    result.algorithm2 = fs::path(algorithm2File).filename().string();
    
    
    try {
        // Load map
        MapData map;
        if (!map.loadFromFile(mapFile, verbose)) {
            result.success = false;
            result.errorMessage = "Failed to load map: " + mapFile;
            return result;
        }
        
        // Check registrations - we need at least 2 algorithms
        {
            std::lock_guard<std::mutex> lock(registryMutex);
            std::cout << "Registered Players: " << playerFactories.size() 
                      << ", TankAlgorithms: " << tankAlgorithmFactories.size() << std::endl;
            if (playerFactories.size() < 2 || tankAlgorithmFactories.size() < 2) {
                result.success = false;
                result.errorMessage = "Need at least 2 algorithms registered, got " + 
                                   std::to_string(playerFactories.size()) + " players and " + 
                                   std::to_string(tankAlgorithmFactories.size()) + " tank algorithms";
                return result;
            }
        }
        
        // Check GameManager registration
        {
            std::lock_guard<std::mutex> lock(registryMutex);
            if (gameManagerFactories.empty()) {
                result.success = false;
                result.errorMessage = "No GameManager registered";
                return result;
            }
        }
        
        // Use the registered factories - algorithm1 for player1, algorithm2 for player2
        GameManagerFactory gameManagerFactory = gameManagerFactories[0];
        PlayerFactory player1Factory = playerFactories[0];  // First registered player
        PlayerFactory player2Factory = playerFactories[1];  // Second registered player
        TankAlgorithmFactory tankAlgorithm1Factory = tankAlgorithmFactories[0];  // First registered algorithm
        TankAlgorithmFactory tankAlgorithm2Factory = tankAlgorithmFactories[1];  // Second registered algorithm
        
        // Run the game using GameRunner
        try {
            GameExecution execution = GameRunner::runSingleGame(
                gameManagerFactory,
                "CompetitiveGame",
                player1Factory,
                tankAlgorithm1Factory,
                result.algorithm1,  // First algorithm name
                result.algorithm2,  // Second algorithm name
                map,
                verbose
            );
            
            // Determine winner
            result.winner = execution.result.winner;
            result.success = true;
            result.rounds = execution.result.rounds;
            
        } catch (const std::exception& e) {
            result.success = false;
            result.errorMessage = "Game execution exception: " + std::string(e.what());
        } catch (...) {
            result.success = false;
            result.errorMessage = "Game execution failed with unknown exception";
        }
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = "Exception: " + std::string(e.what());
    }
    
    return result;
}



// Calculate algorithm pairings for a given map index
std::vector<std::pair<int, int>> calculatePairings(int mapIndex, int numAlgorithms) {
    std::vector<std::pair<int, int>> pairings;
    
    for (int i = 0; i < numAlgorithms; ++i) {
        int opponent = (i + 1 + mapIndex % (numAlgorithms - 1)) % numAlgorithms;
        
        // Avoid duplicate pairings when k = N/2 - 1 (if N is even)
        if (numAlgorithms % 2 == 0 && mapIndex == numAlgorithms / 2 - 1) {
            if (i < opponent) { // Only add each pair once
                pairings.emplace_back(i, opponent);
            }
        } else {
            pairings.emplace_back(i, opponent);
        }
    }
    
    return pairings;
}

// Write competition results to file
void writeCompetitionResults(const std::string& outputPath, 
                           const CompetitiveArgs& config,
                           const std::vector<AlgorithmScore>& algorithmScores) {
    
    std::ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Cannot create output file: " << outputPath << std::endl;
        std::cout << "\n--- COMPETITION RESULTS ---\n";
        std::cout << "game_maps_folder=" << config.gameMapsFolder << std::endl;
        std::cout << "game_manager=" << fs::path(config.gameManager).filename().string() << std::endl;
        std::cout << std::endl;
        
        for (const auto& score : algorithmScores) {
            std::cout << score.name << " " << score.totalScore << std::endl;
        }
        return;
    }
    
    outputFile << "game_maps_folder=" << config.gameMapsFolder << std::endl;
    outputFile << "game_manager=" << fs::path(config.gameManager).filename().string() << std::endl;
    outputFile << std::endl;
    
    for (const auto& score : algorithmScores) {
        outputFile << score.name << " " << score.totalScore << std::endl;
    }
    
    outputFile.close();
    std::cout << "Results written to: " << outputPath << std::endl;
}

int main(int argc, char* argv[]) {
    // Check for competitive mode flag
    if (argc < 2 || std::string(argv[1]) != "-competition") {
        std::cerr << "Usage: " << argv[0] << " -competition game_maps_folder=<dir> game_manager=<file> algorithms_folder=<dir> [num_threads=<n>] [-verbose]\n";
        return 1;
    }
    
    // Declare library handles outside try block so they can be cleaned up in catch
    std::vector<void*> algorithmHandles;
    void* gameManagerHandle = nullptr;
    
    try {
        // Parse arguments (skip the -competition flag)
        char** args = argv + 1;
        CompetitiveArgs config = parseArgs(argc - 1, args);
        
        if (!config.isValid()) {
            std::cerr << "Error: Invalid arguments. All required parameters must be provided.\n";
            return 1;
        }
        
        // Validate paths
        validatePaths(config);
        
        // Collect map files
        std::vector<std::string> mapFiles;
        for (const auto& entry : fs::directory_iterator(config.gameMapsFolder)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                mapFiles.push_back(entry.path().string());
            }
        }
        
        if (mapFiles.empty()) {
            std::cerr << "Error: No map files found in game maps folder: " << config.gameMapsFolder << std::endl;
            return 1;
        }
        
        // Collect algorithm files
        std::vector<std::string> algorithmFiles;
        for (const auto& entry : fs::directory_iterator(config.algorithmsFolder)) {
            if (entry.is_regular_file() && entry.path().extension() == ".so") {
                algorithmFiles.push_back(entry.path().string());
            }
        }
        
        if (algorithmFiles.size() < 2) {
            std::cerr << "Error: Need at least 2 algorithms for competition. Found: " << algorithmFiles.size() << std::endl;
            return 1;
        }
        
        std::cout << "Found " << mapFiles.size() << " map files\n";
        std::cout << "Found " << algorithmFiles.size() << " algorithm files\n";
        std::cout << "Using " << config.numThreads << " thread(s)\n";
        
        // Load all algorithm libraries first
        std::cout << "Loading algorithm libraries..." << std::endl;
        for (const auto& algFile : algorithmFiles) {
            void* handle = dlopen(algFile.c_str(), RTLD_LAZY | RTLD_GLOBAL);
            if (!handle) {
                std::cerr << "Error: Failed to load algorithm: " << algFile << " - " << dlerror() << std::endl;
                return 1;
            }
            algorithmHandles.push_back(handle);
            std::cout << "Loaded algorithm: " << fs::path(algFile).filename().string() << std::endl;
        }
        
        // Load GameManager library
        std::cout << "Loading GameManager library..." << std::endl;
        gameManagerHandle = dlopen(config.gameManager.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (!gameManagerHandle) {
            std::cerr << "Error: Failed to load GameManager: " << config.gameManager << " - " << dlerror() << std::endl;
            // Clean up algorithm handles
            for (void* handle : algorithmHandles) {
                dlclose(handle);
            }
            return 1;
        }
        std::cout << "Loaded GameManager: " << fs::path(config.gameManager).filename().string() << std::endl;
        
        // Wait for registrations
        std::cout << "Waiting for registrations..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Check registrations
        {
            std::lock_guard<std::mutex> lock(registryMutex);
            std::cout << "Registered Players: " << playerFactories.size() << std::endl;
            std::cout << "Registered TankAlgorithms: " << tankAlgorithmFactories.size() << std::endl;
            std::cout << "Registered GameManagers: " << gameManagerFactories.size() << std::endl;
            
            if (playerFactories.size() < 2 || tankAlgorithmFactories.size() < 2 || gameManagerFactories.empty()) {
                std::cerr << "Error: Insufficient registrations. Need at least 2 players, 2 tank algorithms, and 1 game manager." << std::endl;
                // Clean up
                for (void* handle : algorithmHandles) {
                    dlclose(handle);
                }
                dlclose(gameManagerHandle);
                return 1;
            }
        }
        
        // Initialize algorithm scores
        std::map<std::string, AlgorithmScore> algorithmScores;
        for (const auto& algFile : algorithmFiles) {
            std::string name = fs::path(algFile).filename().string();
            algorithmScores[name] = AlgorithmScore(name);
        }
        
        // Run all competitions
        std::vector<CompetitiveGameResult> allResults;
        
                if (config.numThreads == 1) {
            // Single-threaded execution
            for (size_t mapIndex = 0; mapIndex < mapFiles.size(); ++mapIndex) {
                
                // Calculate pairings for this map
                std::vector<std::pair<int, int>> pairings = calculatePairings(mapIndex, algorithmFiles.size());
                
                for (const auto& [algo1Index, algo2Index] : pairings) {
                    
                    auto result = runCompetitiveGame(
                        mapFiles[mapIndex],
                        config.gameManager,
                        algorithmFiles[algo1Index],
                        algorithmFiles[algo2Index],
                        algo1Index,
                        algo2Index,
                        config.verbose
                    );
                    
                    allResults.push_back(result);
                    
                    // Debug: Show each game result
                    std::cout << "GAME RESULT: " << result.mapName << " - " 
                              << fs::path(algorithmFiles[algo1Index]).filename().string() << " vs " 
                              << fs::path(algorithmFiles[algo2Index]).filename().string() 
                              << " -> Winner: ";
                    if (result.winner == 0) {
                        std::cout << "TIE";
                    } else if (result.winner == 1) {
                        std::cout << fs::path(algorithmFiles[algo1Index]).filename().string() << " WINS";
                    } else if (result.winner == 2) {
                        std::cout << fs::path(algorithmFiles[algo2Index]).filename().string() << " WINS";
                    } else {
                        std::cout << "UNKNOWN (" << result.winner << ")";
                    }
                    std::cout << " (Rounds: " << result.rounds << ")" << std::endl;
                    
                    if (result.success) {
                        // Update scores based on game outcome
                        std::string algo1Name = fs::path(algorithmFiles[algo1Index]).filename().string();
                        std::string algo2Name = fs::path(algorithmFiles[algo2Index]).filename().string();
                        
                        if (result.winner == 0) { // Tie
                            algorithmScores[algo1Name].ties++;
                            algorithmScores[algo1Name].totalScore += 1;
                            algorithmScores[algo2Name].ties++;
                            algorithmScores[algo2Name].totalScore += 1;
                        } else if (result.winner == 1) { // Player1 (algo1) wins
                            algorithmScores[algo1Name].wins++;
                            algorithmScores[algo1Name].totalScore += 3;
                            algorithmScores[algo2Name].losses++;
                        } else { // Player2 (algo2) wins
                            algorithmScores[algo2Name].wins++;
                            algorithmScores[algo2Name].totalScore += 3;
                            algorithmScores[algo1Name].losses++;
                        }
                    } else {
                        std::cout << "  GAME FAILED: " << result.errorMessage << std::endl;
                    }
                }
            }
        } else {
            // Multi-threaded execution
            CompetitiveThreadPool pool(config.numThreads);
            std::vector<std::future<CompetitiveGameResult>> futures;
            
            for (size_t mapIndex = 0; mapIndex < mapFiles.size(); ++mapIndex) {
                
                // Calculate pairings for this map
                std::vector<std::pair<int, int>> pairings = calculatePairings(mapIndex, algorithmFiles.size());
                
                for (const auto& [algo1Index, algo2Index] : pairings) {
                    futures.push_back(pool.enqueue([mapFiles, mapIndex, config, algo1Index, algo2Index, algorithmFiles]() {
                        return runCompetitiveGame(
                            mapFiles[mapIndex],
                            config.gameManager,
                            algorithmFiles[algo1Index],
                            algorithmFiles[algo2Index],
                            algo1Index,
                            algo2Index,
                            config.verbose // Use the actual verbose flag from config
                        );
                    }));
                }
            }
            
            // Collect results
            for (auto& future : futures) {
                auto result = future.get();
                allResults.push_back(result);
                
                // Debug: Show each game result
                std::cout << "GAME RESULT: " << result.mapName << " - " 
                          << result.algorithm1 << " vs " << result.algorithm2 
                          << " -> Winner: ";
                if (result.winner == 0) {
                    std::cout << "TIE";
                } else if (result.winner == 1) {
                    std::cout << result.algorithm1 << " WINS";
                } else if (result.winner == 2) {
                    std::cout << result.algorithm2 << " WINS";
                } else {
                    std::cout << "UNKNOWN (" << result.winner << ")";
                }
                std::cout << " (Rounds: " << result.rounds << ")" << std::endl;
                
                if (result.success) {
                    // Update scores based on game outcome
                    if (result.winner == 0) { // Tie
                        algorithmScores[result.algorithm1].ties++;
                        algorithmScores[result.algorithm1].totalScore += 1;
                        algorithmScores[result.algorithm2].ties++;
                        algorithmScores[result.algorithm2].totalScore += 1;
                    } else if (result.winner == 1) { // Player1 (algorithm1) wins
                        algorithmScores[result.algorithm1].wins++;
                        algorithmScores[result.algorithm1].totalScore += 3;
                        algorithmScores[result.algorithm2].losses++;
                    } else if (result.winner == 2) { // Player2 (algorithm2) wins
                        algorithmScores[result.algorithm2].wins++;
                        algorithmScores[result.algorithm2].totalScore += 3;
                        algorithmScores[result.algorithm1].losses++;
                    }
                } else {
                    std::cout << "  GAME FAILED: " << result.errorMessage << std::endl;
                }
            }
        }
        
        // Convert to vector and sort by score
        std::vector<AlgorithmScore> sortedScores;
        for (auto& [name, score] : algorithmScores) {
            sortedScores.push_back(score);
        }
        
        std::sort(sortedScores.begin(), sortedScores.end(), 
                 [](const AlgorithmScore& a, const AlgorithmScore& b) {
                     // Sort by wins first (descending), then by total score as tiebreaker
                     if (a.wins != b.wins) {
                         return a.wins > b.wins;
                     }
                     return a.totalScore > b.totalScore;
                 });
        
        // Generate output filename
        std::string timestamp = getTimestampString();
        std::string outputPath = fs::path(config.algorithmsFolder) / ("competition_" + timestamp + ".txt");
        
        // Write results
        writeCompetitionResults(outputPath, config, sortedScores);
        
        // Print summary
        std::cout << "\nCompetition Summary:\n";
        std::cout << "  Total maps: " << mapFiles.size() << std::endl;
        std::cout << "  Total algorithms: " << algorithmFiles.size() << std::endl;
        std::cout << "  Total games: " << allResults.size() << std::endl;
        std::cout << "  Successful games: " << std::count_if(allResults.begin(), allResults.end(), [](const auto& r) { return r.success; }) << std::endl;
        std::cout << "  Failed games: " << std::count_if(allResults.begin(), allResults.end(), [](const auto& r) { return !r.success; }) << std::endl;
        
        std::cout << "\nFinal Rankings:\n";
        for (size_t i = 0; i < sortedScores.size(); ++i) {
            const auto& score = sortedScores[i];
            std::cout << "  " << (i + 1) << ". " << score.name << " - " << score.totalScore 
                      << " points (W:" << score.wins << " T:" << score.ties << " L:" << score.losses << ")\n";
        }
        
        // Note: Libraries will be cleaned up by the OS when the program exits
        std::cout << "Competition completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
