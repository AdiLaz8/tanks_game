# Tanks Game - Advanced Programming Assignment 3

## Contributors
- **Jonathan Panov** (ID: 206580102)
- **Adi Lazarovich** (ID: 318772340)

## Project Overview
This project implements a comprehensive tanks battle simulation system with support for both comparative and competitive game modes. The system features dynamic library loading and multi-threading capabilities.

## Project Structure
```
tanks_game/
├── Algorithm/              # Player and TankAlgorithm implementations
│   ├── Player_318772340_206580102.cpp/h     # Player management logic
│   ├── TankAlgorithm_318772340_206580102.cpp/h  # Tank AI with BFS pathfinding
│   ├── MyBattleInfo.cpp/h  # Battle information handling
│   └── Makefile            # Algorithm shared library build
├── GameManager/            # Game logic and board management
│   ├── GameManager_318772340_206580102.cpp/h   # Main game controller
│   ├── Board.cpp/h         # Game board and object management
│   ├── Tank.cpp/h          # Tank entity implementation
│   ├── Shell.cpp/h         # Shell projectile logic
│   ├── Wall.cpp/h          # Wall obstacle implementation
│   ├── MySatelliteView.cpp/h  # Game state view
│   └── Makefile            # GameManager shared library build
├── Simulator/              # Simulation orchestration
│   ├── main_comparative_standalone.cpp  # Comparative mode entry point
│   ├── main_competitive.cpp             # Competitive mode entry point
│   ├── GameRunner.cpp/h    # Single game execution logic
│   ├── Registry.cpp/h      # Dynamic component registration
│   ├── ThreadPool.cpp/h    # Multi-threading support
│   └── Makefile            # Simulator executables build
├── UserCommon/             # Shared utilities (no Makefile per assignment)
│   ├── Position.cpp/h      # 2D coordinate handling
│   └── Direction.cpp/h     # 8-directional movement
├── common/                 # Interface definitions
├── maps/                   # Game map files (map1.txt, map2.txt, map3.txt)
└── Makefile               # Unified build system
```

## Building the Project
The project uses a unified Makefile system that builds all components in the correct dependency order:

```bash
make                    # Build all components (default)
make clean             # Clean all build artifacts
make install           # Install simulators to root directory
make status            # Show current build status
make help              # Display available targets
```

Individual components can be built separately:
```bash
make usercommon        # Build UserCommon objects
make algorithm         # Build Algorithm shared library
make gamemanager       # Build GameManager shared library
make simulator         # Build Simulator executables
```

## Running Simulations

### Comparative Mode
Runs multiple GameManagers against the same algorithm pair on a single map:
```bash
./simulator_318772340_206580102 -comparative game_map=<map_file> game_managers_folder=<folder> algorithm1=<algo1.so> algorithm2=<algo2.so> [num_threads=<num>] [-verbose]
```

Example:
```bash
./simulator_318772340_206580102 -comparative game_map=../maps/map1.txt game_managers_folder=../GameManagers algorithm1=../Algorithms/Algorithm_318772340_206580102.so algorithm2=../Algorithms/Algorithm_318772340_206580102.so num_threads=2 -verbose
```

### Competitive Mode
Tournament-style competition between multiple algorithms across multiple maps:
```bash
./simulator_competitive_318772340_206580102 -competition game_maps_folder=<folder> game_manager=<gamemanager.so> algorithms_folder=<folder> [num_threads=<num>] [-verbose]
```

Example:
```bash
./simulator_competitive_318772340_206580102 -competition game_maps_folder=../maps game_manager=../GameManagers/GameManager_318772340_206580102.so algorithms_folder=../Algorithms num_threads=2 -verbose
```

## Implementation Details

### Multi-Threading Support
- **Comparative Mode**: Supports configurable number of worker threads for parallel GameManager execution
- **Thread Safety**: Implements mutex-based synchronization for dynamic library loading and registry access
- **Thread Pool**: Custom ThreadPool implementation with future-based result collection

### Dynamic Library Loading
- **Runtime Loading**: Uses dlopen/dlclose for dynamic component loading
- **Factory Pattern**: Automatic registration system for Players, TankAlgorithms, and GameManagers
- **Registry System**: Thread-safe global registry for component discovery

### Algorithm Implementation
- **Player 1 Strategy**: Aggressive pathfinding with BFS algorithm, target acquisition, and threat assessment
- **Player 2 Strategy**: Defensive positioning with mine avoidance and opportunistic shooting
- **Battle Info**: Comprehensive battlefield awareness including enemy positions, mine locations, and shell tracking

### Game Manager Features
- **Collision Detection**: Tank-tank, tank-mine, and shell collision handling
- **Wrapping Movement**: Toroidal board topology with position wrapping
- **Game Rules**: Configurable max steps, shell limits, and victory conditions
- **State Tracking**: Complete game state preservation for result analysis

### Output System
- **Simple Output**: Uses std::cout and std::cerr for essential messages
- **No Configuration**: No configuration files needed
- **Assignment Compliant**: Avoids shared library conflicts
- **Clean Operation**: No static variable dependencies between .so files

## Map Format
Maps are text files with the following format:
```
MaxSteps = <number>
NumShells = <number>
<width>x<height>
<map_data>
```

Where map_data uses:
- `#` - Wall (HP 2)
- `@` - Wall (HP 1)  
- `*` - Mine
- `1` - Player 1 tank starting position
- `2` - Player 2 tank starting position
- ` ` - Empty space

## Output Files
- **Comparative Mode**: `comparative_results_<timestamp>.txt` with GameManager comparison results
- **Competitive Mode**: `competitive_results_<timestamp>.txt` with tournament leaderboard
- **Log Files**: `tanks_game.log` (or configured filename) with detailed execution logs

## Technical Requirements
- **C++20** standard compilation
- **POSIX** threading support (pthread)
- **Dynamic linking** support (libdl)
- **Linux/Unix** environment

## Notes
- All shared libraries (.so files) must be compiled before running simulations
- The system supports multiple tanks per player (configurable in map files)
- Thread count in comparative mode should not exceed available CPU cores for optimal performance
- Logging can be disabled by setting LOG_LEVEL=OFF in configuration files