#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "ComparativeSimulator.h"
#include "CompetitiveSimulator.h"
#include "SimulationConfig.h"

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n\n";
    std::cout << "Tank Game Simulator - Comparative & Competitive Modes\n\n";
    std::cout << "Options:\n";
    std::cout << "  --mode MODE               Simulation mode: comparative or competitive (default: comparative)\n";
    std::cout << "\n";
    std::cout << "Comparative Mode Options:\n";
    std::cout << "  --game-managers-folder DIR  Folder containing GameManager .so files\n";
    std::cout << "  --game-map FILE             Single map file to test all GameManagers on\n";
    std::cout << "  --algorithm1 FILE           Algorithm 1 .so file\n";
    std::cout << "  --algorithm2 FILE           Algorithm 2 .so file\n";
    std::cout << "  --threads N                 Number of worker threads (default: 4)\n";
    std::cout << "\n";
    std::cout << "Competitive Mode Options:\n";
    std::cout << "  --game-managers FILES       Comma-separated list of GameManager .so files\n";
    std::cout << "  --algorithms FILES          Comma-separated list of Algorithm .so files\n";
    std::cout << "  --maps FILES                Comma-separated list of map files\n";
    std::cout << "  --threads N                 Number of worker threads (default: 4)\n";
    std::cout << "  --max-steps N               Maximum steps per game (default: 1000)\n";
    std::cout << "  --num-shells N              Number of shells per tank (default: 10)\n";
    std::cout << "  --output FILE               Output file for results (default: results.json)\n";
    std::cout << "  --tournament-format FMT     Tournament format: rr, se, de (default: rr)\n";
    std::cout << "  --games-per-matchup N       Games per competitor pair (default: 1)\n";
    std::cout << "\n";
    std::cout << "General Options:\n";
    std::cout << "  --verbose                   Enable verbose logging\n";
    std::cout << "  --help                      Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  # Comparative mode:\n";
    std::cout << "  " << programName << " --mode comparative \\\n";
    std::cout << "                --game-managers-folder ./GameManagers \\\n";
    std::cout << "                --game-map map1.txt \\\n";
    std::cout << "                --algorithm1 Algorithm1.so \\\n";
    std::cout << "                --algorithm2 Algorithm2.so \\\n";
    std::cout << "                --threads 8\n\n";
    std::cout << "  # Competitive mode:\n";
    std::cout << "  " << programName << " --mode competitive \\\n";
    std::cout << "                --game-managers GameManager1.so \\\n";
    std::cout << "                --algorithms Algorithm1.so,Algorithm2.so,Algorithm3.so \\\n";
    std::cout << "                --maps map1.txt,map2.txt \\\n";
    std::cout << "                --tournament-format rr --games-per-matchup 3\n\n";
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

struct ExtendedConfig {
    SimulationConfig base;
    std::string tournamentFormat = "rr";
    size_t gamesPerMatchup = 1;
    // Comparative mode specific fields
    std::string gameManagersFolder;
    std::string gameMapFile;
    std::string algorithm1File;
    std::string algorithm2File;
};

ExtendedConfig parseCommandLine(int argc, char* argv[]) {
    ExtendedConfig extConfig;
    SimulationConfig& config = extConfig.base;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            exit(0);
        } else if (arg == "--mode" && i + 1 < argc) {
            std::string mode = argv[++i];
            if (mode == "comparative") {
                config.mode = SimulationConfig::COMPARATIVE;
            } else if (mode == "competitive") {
                config.mode = SimulationConfig::COMPETITIVE;
            } else {
                std::cerr << "Error: Invalid mode '" << mode << "'. Use 'comparative' or 'competitive'.\n";
                exit(1);
            }
        } else if (arg == "--game-managers" && i + 1 < argc) {
            config.gameManagerSoFiles = split(argv[++i], ',');
        } else if (arg == "--game-managers-folder" && i + 1 < argc) {
            extConfig.gameManagersFolder = argv[++i];
        } else if (arg == "--game-map" && i + 1 < argc) {
            extConfig.gameMapFile = argv[++i];
        } else if (arg == "--algorithm1" && i + 1 < argc) {
            extConfig.algorithm1File = argv[++i];
        } else if (arg == "--algorithm2" && i + 1 < argc) {
            extConfig.algorithm2File = argv[++i];
        } else if (arg == "--algorithms" && i + 1 < argc) {
            config.algorithmSoFiles = split(argv[++i], ',');
        } else if (arg == "--maps" && i + 1 < argc) {
            config.mapFiles = split(argv[++i], ',');
        } else if (arg == "--threads" && i + 1 < argc) {
            config.numThreads = std::stoul(argv[++i]);
        } else if (arg == "--max-steps" && i + 1 < argc) {
            config.maxSteps = std::stoul(argv[++i]);
        } else if (arg == "--num-shells" && i + 1 < argc) {
            config.numShells = std::stoul(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            config.outputFile = argv[++i];
        } else if (arg == "--tournament-format" && i + 1 < argc) {
            extConfig.tournamentFormat = argv[++i];
        } else if (arg == "--games-per-matchup" && i + 1 < argc) {
            extConfig.gamesPerMatchup = std::stoul(argv[++i]);
        } else if (arg == "--verbose") {
            config.verbose = true;
        } else {
            std::cerr << "Error: Unknown argument '" << arg << "'\n";
            printUsage(argv[0]);
            exit(1);
        }
    }
    
    return extConfig;
}

void validateConfig(const ExtendedConfig& extConfig) {
    const SimulationConfig& config = extConfig.base;
    
    if (config.numThreads == 0) {
        std::cerr << "Error: Number of threads must be greater than 0\n";
        exit(1);
    }
    
    if (config.mode == SimulationConfig::COMPARATIVE) {
        // Validate comparative mode requirements
        if (extConfig.gameManagersFolder.empty()) {
            std::cerr << "Error: No GameManager folder specified. Use --game-managers-folder\n";
            exit(1);
        }
        
        if (extConfig.gameMapFile.empty()) {
            std::cerr << "Error: No game map file specified. Use --game-map\n";
            exit(1);
        }
        
        if (extConfig.algorithm1File.empty()) {
            std::cerr << "Error: No algorithm1 file specified. Use --algorithm1\n";
            exit(1);
        }
        
        if (extConfig.algorithm2File.empty()) {
            std::cerr << "Error: No algorithm2 file specified. Use --algorithm2\n";
            exit(1);
        }
    } else {
        // Validate competitive mode requirements
        if (config.gameManagerSoFiles.empty()) {
            std::cerr << "Error: No GameManager .so files specified. Use --game-managers\n";
            exit(1);
        }
        
        if (config.algorithmSoFiles.empty()) {
            std::cerr << "Error: No Algorithm .so files specified. Use --algorithms\n";
            exit(1);
        }
        
        if (config.mapFiles.empty()) {
            std::cerr << "Error: No map files specified. Use --maps\n";
            exit(1);
        }
    }
}

CompetitiveConfig createCompetitiveConfig(const ExtendedConfig& extConfig) {
    CompetitiveConfig compConfig;
    
    if (extConfig.tournamentFormat == "rr") {
        compConfig.format = CompetitiveConfig::ROUND_ROBIN;
    } else if (extConfig.tournamentFormat == "se") {
        compConfig.format = CompetitiveConfig::SINGLE_ELIMINATION;
    } else if (extConfig.tournamentFormat == "de") {
        compConfig.format = CompetitiveConfig::DOUBLE_ELIMINATION;
    } else {
        std::cerr << "Error: Invalid tournament format '" << extConfig.tournamentFormat 
                  << "'. Use 'rr', 'se', or 'de'.\n";
        exit(1);
    }
    
    compConfig.gamesPerMatchup = extConfig.gamesPerMatchup;
    compConfig.useMultipleMaps = true;
    compConfig.parallelizeMatches = true;
    
    return compConfig;
}

int main(int argc, char* argv[]) {
    std::cout << "🏆 Tank Game Simulator v2.0 - Comparative & Competitive\n";
    std::cout << "========================================================\n\n";
    
    try {
        // Parse command line arguments
        ExtendedConfig extConfig = parseCommandLine(argc, argv);
        SimulationConfig& config = extConfig.base;
        validateConfig(extConfig);
        
        // Print configuration
        std::cout << "Configuration:\n";
        std::cout << "  Mode: " << (config.mode == SimulationConfig::COMPARATIVE ? "Comparative" : "Competitive") << "\n";
        
        if (config.mode == SimulationConfig::COMPARATIVE) {
            std::cout << "  GameManagers Folder: " << extConfig.gameManagersFolder << "\n";
            std::cout << "  Game Map: " << extConfig.gameMapFile << "\n";
            std::cout << "  Algorithm 1: " << extConfig.algorithm1File << "\n";
            std::cout << "  Algorithm 2: " << extConfig.algorithm2File << "\n";
        } else {
            std::cout << "  GameManagers: ";
            for (const auto& gm : config.gameManagerSoFiles) std::cout << gm << " ";
            std::cout << "\n  Algorithms: ";
            for (const auto& alg : config.algorithmSoFiles) std::cout << alg << " ";
            std::cout << "\n  Maps: ";
            for (const auto& map : config.mapFiles) std::cout << map << " ";
            std::cout << "\n  Max Steps: " << config.maxSteps;
            std::cout << "\n  Num Shells: " << config.numShells;
            std::cout << "\n  Output: " << config.outputFile;
            std::cout << "\n  Tournament Format: " << extConfig.tournamentFormat;
            std::cout << "\n  Games per Matchup: " << extConfig.gamesPerMatchup;
        }
        std::cout << "\n  Threads: " << config.numThreads;
        std::cout << "\n  Verbose: " << (config.verbose ? "Yes" : "No") << "\n\n";
        
        // Run simulation based on mode
        if (config.mode == SimulationConfig::COMPARATIVE) {
            ComparativeSimulator simulator;
            simulator.configure(
                extConfig.gameManagersFolder,
                extConfig.gameMapFile,
                extConfig.algorithm1File,
                extConfig.algorithm2File,
                config.numThreads
            );
            simulator.runComparative();
        } else {
            CompetitiveSimulator simulator;
            simulator.configure(config);
            simulator.setCompetitiveConfig(createCompetitiveConfig(extConfig));
            simulator.runTournament();
            simulator.exportResults(config.outputFile);
        }
        
        std::cout << "\n🎉 Simulation completed successfully!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}