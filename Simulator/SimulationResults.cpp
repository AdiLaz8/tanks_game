#include "SimulationResults.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>

void SimulationResults::addGameExecution(GameExecution&& execution) {
    gameExecutions.push_back(std::move(execution));
    totalGames++;
    if (execution.success) {
        successfulGames++;
    }
}

void SimulationResults::calculateStats() {
    managerStats.clear();
    
    // Collect stats for each GameManager
    for (const auto& execution : gameExecutions) {
        if (!execution.success) continue;
        
        ManagerStats& stats = managerStats[execution.gameManagerName];
        stats.name = execution.gameManagerName;
        stats.totalGames++;
        stats.totalTime += execution.executionTime;
        
        if (execution.result.winner == 1 || execution.result.winner == 2) {
            stats.wins++;
        } else {
            stats.ties++;
        }
    }
    
    // Calculate statistics
    for (auto& [name, stats] : managerStats) {
        if (stats.totalGames > 0) {
            stats.averageTimeMs = static_cast<double>(stats.totalTime.count()) / stats.totalGames;
            stats.winRate = static_cast<double>(stats.wins) / stats.totalGames;
            stats.losses = stats.totalGames - stats.wins - stats.ties;
        }
    }
}

void SimulationResults::exportToJson(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open output file: " << filename << std::endl;
        return;
    }
    
    file << "{\n";
    file << "  \"totalGames\": " << totalGames << ",\n";
    file << "  \"successfulGames\": " << successfulGames << ",\n";
    file << "  \"totalSimulationTimeMs\": " << totalSimulationTime.count() << ",\n";
    
    // Manager statistics
    file << "  \"managerStats\": {\n";
    bool first = true;
    for (const auto& [name, stats] : managerStats) {
        if (!first) file << ",\n";
        first = false;
        
        file << "    \"" << name << "\": {\n";
        file << "      \"totalGames\": " << stats.totalGames << ",\n";
        file << "      \"wins\": " << stats.wins << ",\n";
        file << "      \"ties\": " << stats.ties << ",\n";
        file << "      \"losses\": " << stats.losses << ",\n";
        file << "      \"winRate\": " << std::fixed << std::setprecision(3) << stats.winRate << ",\n";
        file << "      \"averageTimeMs\": " << std::fixed << std::setprecision(2) << stats.averageTimeMs << "\n";
        file << "    }";
    }
    file << "\n  },\n";
    
    // Individual game results
    file << "  \"gameExecutions\": [\n";
    first = true;
    for (const auto& execution : gameExecutions) {
        if (!first) file << ",\n";
        first = false;
        
        file << "    {\n";
        file << "      \"gameManager\": \"" << execution.gameManagerName << "\",\n";
        file << "      \"algorithm\": \"" << execution.algorithmName << "\",\n";
        file << "      \"map\": \"" << execution.mapName << "\",\n";
        file << "      \"success\": " << (execution.success ? "true" : "false") << ",\n";
        file << "      \"executionTimeMs\": " << execution.executionTime.count() << ",\n";
        
        if (execution.success) {
            file << "      \"winner\": " << execution.result.winner << ",\n";
            file << "      \"rounds\": " << execution.result.rounds << ",\n";
            file << "      \"reason\": " << static_cast<int>(execution.result.reason) << "\n";
        } else {
            file << "      \"error\": \"" << execution.errorMessage << "\"\n";
        }
        
        file << "    }";
    }
    file << "\n  ]\n";
    file << "}\n";
    
    std::cout << "Results exported to: " << filename << std::endl;
}

void SimulationResults::printSummary() const {
    std::cout << "\n╔══════════════════════════════════════╗\n";
    std::cout << "║           SIMULATION SUMMARY         ║\n";
    std::cout << "╠══════════════════════════════════════╣\n";
    std::cout << "║ Total Games: " << std::setw(23) << totalGames << " ║\n";
    std::cout << "║ Successful: " << std::setw(24) << successfulGames << " ║\n";
    std::cout << "║ Failed: " << std::setw(28) << (totalGames - successfulGames) << " ║\n";
    std::cout << "║ Total Time: " << std::setw(20) << totalSimulationTime.count() << " ms ║\n";
    std::cout << "╚══════════════════════════════════════╝\n\n";
    
    if (!managerStats.empty()) {
        std::cout << "╔═══════════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                              GAMEMANAGER COMPARISON                       ║\n";
        std::cout << "╠═══════════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ Manager Name          │ Games │ Wins │ Ties │ Win Rate │ Avg Time (ms) ║\n";
        std::cout << "╠═══════════════════════════════════════════════════════════════════════════╣\n";
        
        std::vector<std::pair<std::string, ManagerStats>> sortedStats(managerStats.begin(), managerStats.end());
        std::sort(sortedStats.begin(), sortedStats.end(), 
                 [](const auto& a, const auto& b) { return a.second.winRate > b.second.winRate; });
        
        for (const auto& [name, stats] : sortedStats) {
            std::cout << "║ " << std::setw(20) << std::left << name << " │ "
                     << std::setw(5) << std::right << stats.totalGames << " │ "
                     << std::setw(4) << stats.wins << " │ "
                     << std::setw(4) << stats.ties << " │ "
                     << std::setw(7) << std::fixed << std::setprecision(1) << (stats.winRate * 100) << "% │ "
                     << std::setw(13) << std::fixed << std::setprecision(1) << stats.averageTimeMs << " ║\n";
        }
        std::cout << "╚═══════════════════════════════════════════════════════════════════════════╝\n";
    }
    
    if (gameExecutions.size() <= 20) {
        std::cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                        INDIVIDUAL GAMES                       ║\n";
        std::cout << "╠════════════════════════════════════════════════════════════════╣\n";
        
        for (const auto& execution : gameExecutions) {
            std::cout << "║ " << std::setw(15) << std::left << execution.gameManagerName 
                     << " vs " << std::setw(10) << execution.algorithmName
                     << " on " << std::setw(8) << execution.mapName;
            
            if (execution.success) {
                std::string result = (execution.result.winner == 0) ? "TIE" : 
                                   (execution.result.winner == 1) ? "P1" : "P2";
                std::cout << " → " << std::setw(3) << result 
                         << " (" << std::setw(4) << execution.executionTime.count() << "ms)";
            } else {
                std::cout << " → FAIL";
            }
            std::cout << " ║\n";
        }
        std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
    }
}