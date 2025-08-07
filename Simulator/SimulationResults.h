#ifndef SIMULATOR_SIMULATION_RESULTS_H
#define SIMULATOR_SIMULATION_RESULTS_H

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include "../common/GameResult.h"

struct GameExecution {
    std::string gameManagerName;
    std::string algorithmName;
    std::string mapName;
    std::string player1Name;
    std::string player2Name;
    GameResult result;
    std::chrono::milliseconds executionTime;
    bool success = true;
    std::string errorMessage;
};

struct ManagerStats {
    std::string name;
    size_t totalGames = 0;
    size_t wins = 0;
    size_t ties = 0;
    size_t losses = 0;
    std::chrono::milliseconds totalTime{0};
    double averageTimeMs = 0.0;
    double winRate = 0.0;
};

struct SimulationResults {
    std::vector<GameExecution> gameExecutions;
    std::map<std::string, ManagerStats> managerStats;
    std::chrono::milliseconds totalSimulationTime{0};
    size_t totalGames = 0;
    size_t successfulGames = 0;
    
    void addGameExecution(GameExecution&& execution);
    void calculateStats();
    void exportToJson(const std::string& filename) const;
    void printSummary() const;
};

#endif // SIMULATOR_SIMULATION_RESULTS_H