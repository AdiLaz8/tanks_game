#ifndef SIMULATOR_SIMULATION_CONFIG_H
#define SIMULATOR_SIMULATION_CONFIG_H

#include <string>
#include <vector>

struct SimulationConfig {
    enum Mode {
        COMPARATIVE,
        COMPETITIVE
    };
    
    Mode mode = COMPARATIVE;
    std::vector<std::string> gameManagerSoFiles;
    std::vector<std::string> algorithmSoFiles;
    std::vector<std::string> mapFiles;
    size_t numThreads = 4;
    size_t maxSteps = 1000;
    size_t numShells = 10;
    std::string outputFile = "results.json";
    bool verbose = false;
};

#endif // SIMULATOR_SIMULATION_CONFIG_H