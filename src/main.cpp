
#include <iostream>
#include <fstream>
#include <string>
#include "Board.h"
#include "Tank.h"
#include "Wall.h"
#include "Mine.h"
#include "GameManager.h"
#include "Logger.h"


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
    Logger::init("log_config.txt");  // הפעלת לוגים על בסיס הקונפיג

    int width, height;
    file >> width >> height;
    file.ignore();// \n
    Board board(width, height);
    bool tank1=false;
    bool tank2=false;
    for (int y = 0; y < height && file; ++y) {
        std::string line;
        std::getline(file, line);
        while (line.length() < static_cast<size_t>(width)){
            line += ' ';
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
                    board.addObject(new Tank('1', 16, Direction(Direction::L), Position(x, y)), x, y);
                    tank1=true;
                }

            }
            else if (c == '2') {
                if(tank2==false){
                    board.addObject(new Tank('2', 16, Direction(Direction::R), Position(x, y)), x, y);
                    tank2=true;
                }

            }
            else{
                continue;
            }
    }
    }
    file.close();
    std::cout << "✅ Board loaded successfully." << std::endl;
    std::cout << "📦 Objects on board:" << std::endl;

    for (int y = 0; y < board.getHeight(); ++y) {
        for (int x = 0; x < board.getWidth(); ++x) {
            const CellSlot& slot = board.getSlot(x, y);
            const auto& objects = slot.getAll();
            for (Cell* obj : objects) {
                std::cout << "[" << x << "," << y << "] "
                        << obj->toString() << std::endl;
            }
        }
    }


// for (int y = 0; y < board.getHeight(); ++y) {
//     for (int x = 0; x < board.getWidth(); ++x) {
//         const CellSlot& slot = board.getSlot(x, y);
//         const std::vector<Cell*>& objects = slot.getAll();

//         if (!objects.empty()) {
//             Cell* obj = objects.back();
//             char symbol = obj->getSymbol();
//             std::cout << symbol;

//             // בדיקה: איזה אובייקט זה באמת?
//             if (dynamic_cast<Tank*>(obj)) {
//                 std::cout << "(T)";
//             } else if (dynamic_cast<Wall*>(obj)) {
//                 std::cout << "(W)";
//             } else if (dynamic_cast<Mine*>(obj)) {
//                 std::cout << "(M)";
//             } else if (dynamic_cast<Shell*>(obj)) {
//                 std::cout << "(S)";
//             } else {
//                 std::cout << "(?)";  // משהו לא צפוי
//             }

//         } else {
//             std::cout << ' ';
//         }
//     }
//     std::cout << '\n';
// }
    GameManager game(board);
    game.gameLoop();

    // 📤 הדפסת קובץ פלט
    std::ifstream outFile("game_output.txt");
    if (!outFile) {
        std::cerr << "Failed to open game_output.txt" << std::endl;
        return 1;
    }

    std::cout << "\n📄 Game Output:\n";
    std::string line;
    while (std::getline(outFile, line)) {
        std::cout << line << std::endl;
    }


    outFile.close();
        // 📤 הדפסת debug_log.txt
    // std::ifstream debugFile("debug_log.txt");
    // if (!debugFile) {
    //     std::cerr << "Failed to open debug_log.txt" << std::endl;
    //     return 1;
    // }

    // std::cout << "\n🪵 Debug Log:\n";
    // std::string debugLine;
    // while (std::getline(debugFile, debugLine)) {
    //     std::cout << debugLine << std::endl;
    // }

    // debugFile.close();

    Logger::shutdown(); // סגירת קובץ הלוג



    return 0;
}
