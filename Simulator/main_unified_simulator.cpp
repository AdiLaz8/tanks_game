#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [-comparative|-competition] [args...]\n";
        std::cerr << "\nComparative mode:\n";
        std::cerr << "  " << argv[0] << " -comparative game_map=<file> game_managers_folder=<dir> algorithm1=<file> algorithm2=<file> [num_threads=<n>] [-verbose]\n";
        std::cerr << "\nCompetitive mode:\n";
        std::cerr << "  " << argv[0] << " -competition game_maps_folder=<dir> game_manager=<file> algorithms_folder=<dir> [num_threads=<n>] [-verbose]\n";
        return 1;
    }
    
    std::string mode = argv[1];
    
    if (mode == "-comparative") {
        // Call comparative mode main function
        extern int main_comparative(int argc, char* argv[]);
        return main_comparative(argc, argv);
    } else if (mode == "-competition") {
        // Call competitive mode main function  
        extern int main_competitive(int argc, char* argv[]);
        return main_competitive(argc, argv);
    } else {
        std::cerr << "Error: Invalid mode '" << mode << "'. Use -comparative or -competition.\n";
        return 1;
    }
}
