#include <iostream>
#include <fstream>
#include <string>
#include "Board.h"
#include "Tank.h"
#include "Wall.h"
#include "Mine.h"
#include "GameManager.h"
#include "Logger.h"
#include "MyPlayerFactory.h"
#include "MyTankAlgorithmFactory.h"
#include <set> 

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <map_file>" << std::endl;
        return 1;
    }
    std::string inputFileName = argv[1];
    MyPlayerFactory playerFactory;
    MyTankAlgorithmFactory tankFactory;
    GameManager manager(playerFactory, tankFactory); 
    manager.readBoard(inputFileName);
    manager.gameLoop();
    return 0;
}