#ifndef SIMULATOR_COMPARATIVE_SIMULATOR_H
#define SIMULATOR_COMPARATIVE_SIMULATOR_H

#include <vector>
#include <memory>
#include <future>
#include <map>
#include <set>
#include <mutex>
#include "SimulationConfig.h"
#include "SimulationResults.h"
#include "DynamicLoader.h"
#include "ThreadPool.h"
#include "GameRunner.h"

// Structure to represent a unique game result for grouping
struct ComparativeGameResult {
    int winner;
    GameResult::Reason reason;
    size_t rounds;
    std::vector<size_t> remaining_tanks;
    std::string finalMapState; // Serialized final map state
    
    // For grouping identical results
    bool operator<(const ComparativeGameResult& other) const;
    bool operator==(const ComparativeGameResult& other) const;
};

struct ComparativeExecution {
    std::string gameManagerName;
    ComparativeGameResult result;
    std::string finalMapState;
    bool success;
    std::string errorMessage;
};

class ComparativeSimulator {
private:
    DynamicLoader loader;
    std::unique_ptr<ThreadPool> threadPool;
    std::string gameManagersFolder;
    std::string gameMapFile;
    std::string algorithm1File;
    std::string algorithm2File;
    size_t numThreads;
    bool verbose;
    
    // Loaded factories
    std::vector<GameManagerFactory> gameManagerFactories;
    std::vector<std::string> gameManagerNames;
    PlayerFactory playerFactory;
    TankAlgorithmFactory algorithm1Factory;
    TankAlgorithmFactory algorithm2Factory;
    
    // Thread safety for concurrent factory usage
    mutable std::mutex factoryMutex;
    
    // Results grouped by identical game outcomes
    std::map<ComparativeGameResult, std::vector<std::string>> resultGroups;
    
public:
    ComparativeSimulator();
    ~ComparativeSimulator();
    
    // Configuration - Updated for comparative mode requirements
    void configure(const std::string& gameManagersFolder,
                  const std::string& gameMapFile,
                  const std::string& algorithm1File,
                  const std::string& algorithm2File,
                  size_t numThreads,
                  bool verbose = false);
    
    // Execution
    void runComparative();
    
private:
    void loadGameManagerLibraries();
    void loadAlgorithmLibraries();
    void validateConfiguration() const;
    void setupThreadPool();
    std::vector<std::future<ComparativeExecution>> queueAllGameManagers();
    ComparativeExecution runSingleGame(size_t gmIdx, const MapData& map);
    void writeResultsToFile(const std::string& outputFile) const;
    std::string generateTimestampedFilename() const;
    std::string serializeMapState(const SatelliteView* satelliteView) const;
    std::string getGameResultMessage(const ComparativeGameResult& result) const;
};

#endif // SIMULATOR_COMPARATIVE_SIMULATOR_H