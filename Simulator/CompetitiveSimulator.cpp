#include "CompetitiveSimulator.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>

CompetitiveSimulator::CompetitiveSimulator() = default;

CompetitiveSimulator::~CompetitiveSimulator() = default;

void CompetitiveSimulator::configure(const SimulationConfig& config) {
    std::cout << "Configuring Competitive Simulator..." << std::endl;
    
    // Load libraries
    loadLibraries(config);
    
    // Load maps
    loadMaps(config.mapFiles);
    
    // Setup thread pool
    setupThreadPool(config.numThreads);
    
    // Set up competitive configuration
    CompetitiveConfig compConfig;
    compConfig.format = CompetitiveConfig::ROUND_ROBIN; // Default
    compConfig.useMultipleMaps = true;
    compConfig.gamesPerMatchup = 1;
    compConfig.parallelizeMatches = true;
    
    // Use first GameManager for competitive mode
    if (!config.gameManagerSoFiles.empty()) {
        compConfig.gameManagerSo = config.gameManagerSoFiles[0];
    }
    
    setCompetitiveConfig(compConfig);
    
    // Setup tournament
    setupTournament();
    
    // Validate everything is ready
    validateConfiguration();
    
    std::cout << "Competitive configuration complete!" << std::endl;
}

void CompetitiveSimulator::setCompetitiveConfig(const CompetitiveConfig& compConfig) {
    competitiveConfig = compConfig;
}

void CompetitiveSimulator::loadMaps(const std::vector<std::string>& mapFiles) {
    std::cout << "Loading maps for tournament..." << std::endl;
    maps.clear();
    
    for (const auto& mapFile : mapFiles) {
        MapData map;
        if (map.loadFromFile(mapFile)) {
            maps.push_back(std::move(map));
        } else {
            throw std::runtime_error("Failed to load map: " + mapFile);
        }
    }
    
    if (maps.empty()) {
        throw std::runtime_error("No maps loaded successfully");
    }
    
    std::cout << "Loaded " << maps.size() << " maps for tournament" << std::endl;
}

void CompetitiveSimulator::loadLibraries(const SimulationConfig& config) {
    std::cout << "Loading dynamic libraries for competitive mode..." << std::endl;
    
    // Load GameManager library (only need one for competitive mode)
    if (!config.gameManagerSoFiles.empty()) {
        std::vector<std::string> singleGM = {config.gameManagerSoFiles[0]};
        loader.loadGameManagerLibraries(singleGM);
        auto gmFactories = loader.getGameManagerFactories();
        auto gmNames = loader.getGameManagerFactoryNames();
        
        if (!gmFactories.empty()) {
            gameManagerFactory = gmFactories[0];
            gameManagerName = gmNames.empty() ? "GameManager" : gmNames[0];
        }
    }
    
    // Load Algorithm libraries (these will compete against each other)
    if (!config.algorithmSoFiles.empty()) {
        loader.loadAlgorithmLibraries(config.algorithmSoFiles);
        playerFactories = loader.getPlayerFactories();
        algorithmFactories = loader.getTankAlgorithmFactories();
        algorithmNames = loader.getTankAlgorithmFactoryNames();
    }
    
    std::cout << "Loaded 1 GameManager and " << algorithmFactories.size() << " competing algorithms" << std::endl;
}

void CompetitiveSimulator::setupThreadPool(size_t numThreads) {
    // For numThreads >= 2, create numThreads worker threads
    // The main thread will also participate in the work
    size_t workerThreads = (numThreads >= 2) ? numThreads : 0;
    threadPool = std::make_unique<ThreadPool>(workerThreads);
}

void CompetitiveSimulator::setupTournament() {
    std::cout << "Setting up tournament structure..." << std::endl;
    
    Tournament::TournamentType tournamentType;
    switch (competitiveConfig.format) {
        case CompetitiveConfig::ROUND_ROBIN:
            tournamentType = Tournament::ROUND_ROBIN;
            break;
        case CompetitiveConfig::SINGLE_ELIMINATION:
            tournamentType = Tournament::SINGLE_ELIMINATION;
            break;
        case CompetitiveConfig::DOUBLE_ELIMINATION:
            tournamentType = Tournament::DOUBLE_ELIMINATION;
            break;
    }
    
    tournament = std::make_unique<Tournament>(tournamentType);
    tournament->setMaps(maps);
    tournament->setGamesPerMatchup(competitiveConfig.gamesPerMatchup);
    tournament->setUseMultipleMaps(competitiveConfig.useMultipleMaps);
    
    registerCompetitors();
    tournament->generateRounds();
}

void CompetitiveSimulator::registerCompetitors() {
    std::cout << "Registering competitors..." << std::endl;
    
    if (playerFactories.empty()) {
        throw std::runtime_error("Need at least 1 player factory for competitive mode");
    }
    
    // Register each algorithm as a competitor
    // Each competitor uses the same algorithm factory but creates different behaviors for player 1 and 2
    for (size_t i = 0; i < algorithmFactories.size(); ++i) {
        std::string competitorName = algorithmNames.empty() ? 
            "Algorithm" + std::to_string(i) : algorithmNames[i];
        
        // Each competitor uses one algorithm factory that creates algorithms for both players
        tournament->addCompetitor(
            competitorName,
            algorithmFactories[i],     // Algorithm factory for both tanks
            playerFactories[0]         // Player factory (same for all)
        );
    }
    
    std::cout << "Registered " << tournament->getCompetitors().size() << " competitors" << std::endl;
}

void CompetitiveSimulator::validateConfiguration() const {
    if (!gameManagerFactory) {
        throw std::runtime_error("No GameManager factory loaded");
    }
    
    if (playerFactories.empty()) {
        throw std::runtime_error("No Player factories loaded");
    }
    
    if (algorithmFactories.size() < 2) {
        throw std::runtime_error("Need at least 2 TankAlgorithm factories for competitive mode");
    }
    
    if (maps.empty()) {
        throw std::runtime_error("No maps loaded");
    }
    
    if (!threadPool) {
        throw std::runtime_error("ThreadPool not initialized");
    }
    
    if (!tournament) {
        throw std::runtime_error("Tournament not initialized");
    }
    
    if (tournament->getCompetitors().size() < 2) {
        throw std::runtime_error("Need at least 2 competitors for tournament");
    }
}

void CompetitiveSimulator::runTournament() {
    std::cout << "\n🏆 Starting Competitive Tournament 🏆" << std::endl;
    std::cout << "Format: " << (competitiveConfig.format == CompetitiveConfig::ROUND_ROBIN ? "Round Robin" : 
                               competitiveConfig.format == CompetitiveConfig::SINGLE_ELIMINATION ? "Single Elimination" : 
                               "Double Elimination") << std::endl;
    std::cout << "Competitors: " << tournament->getCompetitors().size() << std::endl;
    std::cout << "Total Matches: " << tournament->getTotalMatches() << std::endl;
    
    auto startTime = std::chrono::steady_clock::now();
    
    // Execute all matches
    if (competitiveConfig.parallelizeMatches) {
        auto futures = executeAllMatches();
        
        std::cout << "\nExecuting matches in parallel..." << std::endl;
        
        // Collect results as they complete
        for (auto& future : futures) {
            try {
                GameExecution execution = future.get();
                tournament->recordMatchResult(execution.gameManagerName + "_match", execution);
                results.addGameExecution(std::move(execution));
                
                if (execution.success) {
                    std::cout << "✓ Match completed (" << tournament->getCompletedMatches() 
                             << "/" << tournament->getTotalMatches() << ")" << std::endl;
                } else {
                    std::cout << "✗ Match failed: " << execution.errorMessage << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error collecting match result: " << e.what() << std::endl;
            }
        }
    } else {
        // Execute matches sequentially
        auto pendingMatches = tournament->getAllPendingMatches();
        for (auto* match : pendingMatches) {
            GameExecution execution = executeMatch(match);
            tournament->recordMatchResult(match->id, execution);
            results.addGameExecution(std::move(execution));
            
            std::cout << "Completed match: " << match->id << std::endl;
        }
    }
    
    auto endTime = std::chrono::steady_clock::now();
    results.totalSimulationTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    results.calculateStats();
    
    std::cout << "\n🎉 Tournament Complete! 🎉" << std::endl;
    printTournamentSummary();
}

std::vector<std::future<GameExecution>> CompetitiveSimulator::executeAllMatches() {
    std::vector<std::future<GameExecution>> futures;
    
    // Get all pending matches
    auto pendingMatches = tournament->getAllPendingMatches();
    size_t totalMatches = pendingMatches.size();
    
    if (totalMatches == 0) {
        return futures;
    }
    
    // For round robin, we can execute all matches in parallel
    // For elimination tournaments, we need to execute round by round
    if (competitiveConfig.format == CompetitiveConfig::ROUND_ROBIN) {
        // Calculate how many matches to run in worker threads vs main thread
        size_t workerThreads = threadPool->size();
        size_t matchesForWorkers = std::min(workerThreads, totalMatches);
        size_t matchesForMain = totalMatches - matchesForWorkers;
        
        // Queue matches for worker threads
        for (size_t i = 0; i < matchesForWorkers; ++i) {
            auto future = threadPool->enqueue([this, match = pendingMatches[i]]() -> GameExecution {
                return executeMatch(match);
            });
            futures.push_back(std::move(future));
        }
        
        // Main thread runs remaining matches
        for (size_t i = matchesForWorkers; i < totalMatches; ++i) {
            // Create a packaged_task to wrap the match execution
            auto task = std::packaged_task<GameExecution()>([this, match = pendingMatches[i]]() -> GameExecution {
                return executeMatch(match);
            });
            
            // Get the future from the packaged_task
            futures.push_back(task.get_future());
            
            // Execute the task immediately in the main thread
            task();
        }
        
    } else {
        // For elimination tournaments, execute one round at a time
        while (!tournament->isComplete()) {
            auto roundMatches = tournament->getAllPendingMatches();
            std::vector<std::future<GameExecution>> roundFutures;
            
            size_t roundTotal = roundMatches.size();
            size_t workerThreads = threadPool->size();
            size_t matchesForWorkers = std::min(workerThreads, roundTotal);
            size_t matchesForMain = roundTotal - matchesForWorkers;
            
            // Queue matches for worker threads
            for (size_t i = 0; i < matchesForWorkers; ++i) {
                auto future = threadPool->enqueue([this, match = roundMatches[i]]() -> GameExecution {
                    return executeMatch(match);
                });
                roundFutures.push_back(std::move(future));
            }
            
            // Main thread runs remaining matches
            for (size_t i = matchesForWorkers; i < roundTotal; ++i) {
                // Create a packaged_task to wrap the match execution
                auto task = std::packaged_task<GameExecution()>([this, match = roundMatches[i]]() -> GameExecution {
                    return executeMatch(match);
                });
                
                // Get the future from the packaged_task
                roundFutures.push_back(task.get_future());
                
                // Execute the task immediately in the main thread
                task();
            }
            
            // Wait for current round to complete
            for (auto& future : roundFutures) {
                GameExecution execution = future.get();
                tournament->recordMatchResult(execution.gameManagerName + "_match", execution);
            }
        }
    }
    
    return futures;
}

GameExecution CompetitiveSimulator::executeMatch(Match* match) {
    // Find the map data
    auto mapIt = std::find_if(maps.begin(), maps.end(), 
        [&](const MapData& map) { return map.name == match->mapName; });
    
    if (mapIt == maps.end()) {
        GameExecution execution;
        execution.success = false;
        execution.errorMessage = "Map not found: " + match->mapName;
        return execution;
    }
    
    // In competitive mode, we need to run a match between two different competitors
    // But our current GameRunner only supports one algorithm factory per game
    // For now, use competitor1's algorithm - this needs further redesign for true competition
    TankAlgorithmFactory algorithmFactory = match->competitor1->algorithmFactory;
    
    return GameRunner::runSingleGame(
        gameManagerFactory,
        gameManagerName,
        playerFactories[0], // Player factory (creates both players with different indices)
        algorithmFactory,   // Algorithm factory (creates algorithms for both tanks)
        match->competitor1->name + " vs " + match->competitor2->name,
        *mapIt,
        false // verbose
    );
}

void CompetitiveSimulator::exportResults(const std::string& filename) const {
    results.exportToJson(filename);
}

void CompetitiveSimulator::printTournamentSummary() const {
    tournament->printStandings();
    tournament->printTournamentBracket();
    results.printSummary();
}