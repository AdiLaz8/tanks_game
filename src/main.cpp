#include <iostream>
#include <fstream>
#include <string>
#include "Board.h"
#include "Tank.h"
#include "Wall.h"
#include "Mine.h"
#include "GameManager.h"
#include "Logger.h"
#include <set> 



int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: tanks_game <input_file>" << std::endl;
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file!" << std::endl;
        return 1;
    }
    Logger::init("log_config.txt");
    std::ofstream inputErrorsFile;


    // loading the board from the input file
    int width, height;
    file >> width >> height;
    file.ignore();// \n
    Board board(width, height);
    bool tank1=false;
    bool tank2=false;
    int y = 0;

    for (; y < height; ++y) {
        std::string line;
        if (!std::getline(file, line)) {
            // אם חסרות שורות – נמלא ידנית
            line = std::string(width, ' ');
            if (!inputErrorsFile.is_open()) {
                inputErrorsFile.open("input_errors.txt");
            }
            inputErrorsFile << "Warning: Missing row " << y << ", filled with spaces.\n";
        }
        if (line.length() > static_cast<size_t>(width)) {
            line = line.substr(0, width);
            if (!inputErrorsFile.is_open()) {
                inputErrorsFile.open("input_errors.txt");
            }
            inputErrorsFile << "Warning: Row " << y << " longer than declared width, extra characters ignored.\n";
        }
        while (line.length() < static_cast<size_t>(width)){
            line += ' ';
            if (!inputErrorsFile.is_open()) {
                inputErrorsFile.open("input_errors.txt");
            }
            inputErrorsFile << "Warning: Row " << y << " shorter than declared width, filled with spaces.\n";
        }
        for (int x = 0; x < width; ++x) {
            char c = line[x];
            if (c == ' ') continue;
            if (c == '#') {
                board.addObject(new Wall(), x, y);
            }
            else if (c == '@') {
                board.addObject(new Mine(), x, y);
            }
            else if (c == '1') {
                if(tank1==false){
                    Tank* t1 = new Tank('1', 16, Direction(Direction::L), Position(x, y));
                    board.addObject(t1, x, y);
                    tank1=true;
                } else {
                        if (!inputErrorsFile.is_open()) {
                            inputErrorsFile.open("input_errors.txt");
                        }
                        inputErrorsFile << "Warning: Extra Tank 1 at (" << x << "," << y << ") ignored.\n";
                    }
            }
            else if (c == '2') {
                if(tank2==false){
                    Tank* t2 = new Tank('2', 16, Direction(Direction::R), Position(x, y));
                    board.addObject(t2, x, y);
                    tank2=true;
                }else {
                    if (!inputErrorsFile.is_open()) {
                        inputErrorsFile.open("input_errors.txt");
                    }
                    inputErrorsFile << "Warning: Extra Tank 2 at (" << x << "," << y << ") ignored.\n";
                }
            }
            else{
                if (!inputErrorsFile.is_open()) {
                    inputErrorsFile.open("input_errors.txt");
                }
                inputErrorsFile << "Warning: Unrecognized character '" << c << "' at (" << x << "," << y << "), treated as space.\n";
                continue;
            }
    }
    }

    // אם יש שורות מיותרות מעבר ל־height – לדווח
    std::string extraLine;
    while (std::getline(file, extraLine)) {
        if (!extraLine.empty()) {
            if (!inputErrorsFile.is_open()) {
                inputErrorsFile.open("input_errors.txt");
            }
            inputErrorsFile << "Warning: Extra row beyond declared height ignored.\n";
        }
    }

    if (inputErrorsFile.is_open()) {
        inputErrorsFile.close();
    }

    file.close();
    if (!tank1 || !tank2) {
        std::cerr << "Error: Missing tank(s) in input file!" << std::endl;
        return 1;
    }


    // creating the GameManager and starting the game
    GameManager game(board, std::string(argv[1]));
    game.gameLoop();

    // writing to the output file the entire game turns
    std::string outputFileName = "output_" + std::string(argv[1]);
    std::ifstream outFile(outputFileName);
    if (!outFile) {
        std::cerr << "Failed to open game_output.txt" << std::endl;
        return 1;
    }
    outFile.close();

    // deleting the objects remained in the game board at the end
    std::set<Cell*> uniqueObjects;
    for (int y = 0; y < board.getHeight(); ++y) {
        for (int x = 0; x < board.getWidth(); ++x) {
            const CellSlot& slot = board.getSlot(x, y);
            const auto& objects = slot.getAll();
            for (Cell* obj : objects) {
                uniqueObjects.insert(obj); // only one time for each object
            }
        }
    }
    // GameManager itself deletes the shells, so we delete only objects that are not shells
    for (Cell* obj : uniqueObjects) {
        if (dynamic_cast<Shell*>(obj) == nullptr) {
            delete obj;
        }
    }
    Logger::shutdown(); // closing the logger
    return 0;
}
