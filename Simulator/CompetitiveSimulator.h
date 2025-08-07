#ifndef SIMULATOR_COMPETITIVE_SIMULATOR_H
#define SIMULATOR_COMPETITIVE_SIMULATOR_H

#include <vector>
#include <memory>
#include <future>
#include "SimulationConfig.h"
#include "SimulationResults.h"
#include "DynamicLoader.h"
#include "ThreadPool.h"
#include "GameRunner.h"
#include "Tournament.h"

struct CompetitiveConfig {
    enum TournamentFormat {
        ROUND_ROBIN,
        SINGLE_ELIMINATION,
        DOUBLE_ELIMINATION
    } format = ROUND_ROBIN;
    
    bool useMultipleMaps = true;     // Play on all maps or just one
    size_t gamesPerMatchup = 1;      // Number of games per competitor pair
    bool parallelizeMatches = true;   // Run matches in parallel
    std::string gameManagerSo;        // Single GameManager for competitive mode
};

class CompetitiveSimulator {
private:
    DynamicLoader loader;
    std::unique_ptr<ThreadPool> threadPool;
    std::vector<MapData> maps;
    std::unique_ptr<Tournament> tournament;
    SimulationResults results;
    CompetitiveConfig competitiveConfig;
    
    // Loaded factories
    GameManagerFactory gameManagerFactory;
    std::string gameManagerName;
    std::vector<PlayerFactory> playerFactories;
    std::vector<TankAlgorithmFactory> algorithmFactories;
    std::vector<std::string> algorithmNames;
    
public:
    CompetitiveSimulator();
    ~CompetitiveSimulator();
    
    // Configuration
    void configure(const SimulationConfig& config);
    void setCompetitiveConfig(const CompetitiveConfig& compConfig);
    
    // Loading
    void loadMaps(const std::vector<std::string>& mapFiles);
    void loadLibraries(const SimulationConfig& config);
    
    // Tournament setup
    void setupTournament();
    void registerCompetitors();
    
    // Execution
    void runTournament();
    
    // Results
    const SimulationResults& getResults() const { return results; }
    const Tournament& getTournament() const { return *tournament; }
    void exportResults(const std::string& filename) const;
    void printTournamentSummary() const;
    
private:
    void validateConfiguration() const;
    void setupThreadPool(size_t numThreads);
    std::vector<std::future<GameExecution>> executeAllMatches();
    GameExecution executeMatch(Match* match);
};

#endif // SIMULATOR_COMPETITIVE_SIMULATOR_H