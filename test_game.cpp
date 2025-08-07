#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include "GameManager.h"
#include "MyPlayerFactory.h"
#include "MyTankAlgorithmFactory.h"
#include "Tank.h"
#include "Shell.h"
#include "Wall.h"
#include "Mine.h"
#include "Position.h"
#include "Direction.h"
#include "Algo1.h"
#include "Algo2.h"

/// create a simple board file with given map, rows and cols for testing
void createSimpleBoardFile(const std::string& filename, const std::vector<std::string>& map, size_t rows, size_t cols) {
    std::ofstream out(filename);
    out << "Test Board\n";
    out << "MaxSteps=100\n";
    out << "NumShells=5\n";
    out << "Rows=" << rows << "\n";
    out << "Cols=" << cols << "\n";
    for (size_t i = 0; i < rows; ++i) {
        std::string line = (i < map.size() ? map[i] : "");
        while (line.length() < cols) line += ' ';
        if (line.length() > cols) line = line.substr(0, cols);
        out << line << "\n";
    }
    out.close();
}

void createSimpleBoardFile(const std::string& filename, const std::vector<std::string>& map, size_t rows, size_t cols, size_t maxSteps) {
    std::ofstream out(filename);
    out << "Test Board\n";
    out << "MaxSteps=" << maxSteps << "\n";
    out << "NumShells=5\n";
    out << "Rows=" << rows << "\n";
    out << "Cols=" << cols << "\n";
    for (size_t i = 0; i < rows; ++i) {
        std::string line = (i < map.size() ? map[i] : "");
        while (line.length() < cols) line += ' ';
        if (line.length() > cols) line = line.substr(0, cols);
        out << line << "\n";
    }
    out.close();
}


void removeFile(const std::string& fname) { std::remove(fname.c_str()); }

// 1. create GameManager with PlayerFactory and TankAlgorithmFactory, should not throw
TEST(GameManagerTest, CreateGameManagerNoThrow) {
    MyPlayerFactory pf; MyTankAlgorithmFactory tf;
    EXPECT_NO_THROW(GameManager gm(pf, tf));
}

// 2. create PlayerFactory and check if it creates correct players
TEST(GameManagerTest, PlayerFactoryCreatesCorrectPlayers) {
    MyPlayerFactory pf;
    auto p1 = pf.create(1, 6, 4, 10, 5);
    auto p2 = pf.create(2, 6, 4, 10, 5);
    EXPECT_NE(dynamic_cast<Player1*>(p1.get()), nullptr);
    EXPECT_NE(dynamic_cast<Player2*>(p2.get()), nullptr);
}

// 3. create TankAlgorithmFactory and check if it creates correct algorithms
TEST(GameManagerTest, TankAlgorithmFactoryCreatesCorrectAlgos) {
    MyTankAlgorithmFactory tf;
    auto a1 = tf.create(1, 0);
    auto a2 = tf.create(2, 1);
    EXPECT_NE(dynamic_cast<Algo1*>(a1.get()), nullptr);
    EXPECT_NE(dynamic_cast<Algo2*>(a2.get()), nullptr);
}

// 4. read board with title and objects works correctly
TEST(GameManagerTest, ReadBoardWithTitleAndObjects) {
    std::string fname = "input_t1.txt";
    std::vector<std::string> map = {
        " 1@  #",
        "     #",
        " 2   #",
        "######"
    };
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    gm.readBoard(fname);
    EXPECT_EQ(gm.getBoard().getWidth(), 6);
    EXPECT_EQ(gm.getBoard().getHeight(), 4);
    removeFile(fname);
    std::remove("output_input_t1.txt");
}

// 5. read board with less columns than written
TEST(GameManagerTest, ReadBoardWithShortRow) {
    std::string fname = "input_short.txt";
    std::vector<std::string> map = {"1", "     ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 7); 
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    gm.readBoard(fname);
    EXPECT_EQ(gm.getBoard().getWidth(), 7);
    removeFile(fname);
    std::remove("output_input_short.txt");
}   

// 6. read board with more columns than written
TEST(GameManagerTest, ReadBoardWithLongRow) {
    std::string fname = "input_long.txt";
    std::vector<std::string> map = {"1     zzz", "     ", "2    ", "######"};
    createSimpleBoardFile(fname, map,5,7);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    gm.readBoard(fname);
    EXPECT_EQ(gm.getBoard().getWidth(), 7);
    removeFile(fname);
    std::remove("output_input_long.txt");
}

// 7. invalid character in the board, should not throw and treat as space
TEST(GameManagerTest, ReadBoardWithUnknownChar) {
    std::string fname = "input_badchar.txt";
    std::vector<std::string> map = {"1  $  ", "     ", "2     ", "######"};
    createSimpleBoardFile(fname, map,4,6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    EXPECT_NO_THROW(gm.readBoard(fname));
    removeFile(fname);
    std::remove("output_input_badchar.txt");
}

// 8. no tanks on the board, should exit with error
TEST(GameManagerTest, ReadBoardNoTanks) {
    std::string fname = "input_notanks.txt";
    std::vector<std::string> map = {"      ", "      ", "      ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    EXPECT_EXIT(gm.readBoard(fname), ::testing::ExitedWithCode(1), "");
    removeFile(fname);
    std::remove("output_input_notanks.txt");
}

// 9. only player 1 has tanks, needs to exit
TEST(GameManagerTest, ReadBoardOnlyPlayer1Tanks) {
    std::string fname = "input_p1tanks.txt";
    std::vector<std::string> map = {"1     ", "      ", "1     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    EXPECT_EXIT(gm.readBoard(fname), ::testing::ExitedWithCode(1), "");
    removeFile(fname);
    std::remove("output_input_p1tanks.txt");
}

// 10. only player 2 has tanks, needs to exit
TEST(GameManagerTest, ReadBoardOnlyPlayer2Tanks) {
    std::string fname = "input_p2tanks.txt";
    std::vector<std::string> map = {"2     ", "      ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    EXPECT_EXIT(gm.readBoard(fname), ::testing::ExitedWithCode(1), "");
    removeFile(fname);
    std::remove("output_input_p2tanks.txt");
}

// 11. tests reading board with missing row
TEST(GameManagerTest, ReadBoardMissingRow) {
    std::string fname = "input_missingrow.txt";
    std::vector<std::string> map = {
        "1     ",
        "  2  ",
        "######"
    };
    int rows = 4; 
    int cols = 6;
    createSimpleBoardFile(fname, map, rows, cols);

    MyPlayerFactory pf; 
    MyTankAlgorithmFactory tf; 
    GameManager gm(pf, tf);

    EXPECT_NO_THROW(gm.readBoard(fname));

    // make sure the board is initialized correctly
    for (int x = 0; x < cols; ++x) {
        EXPECT_EQ(gm.getBoard().getSlot(x, 3).getTank(), nullptr);
        EXPECT_EQ(gm.getBoard().getSlot(x, 3).getWall(), nullptr);
        EXPECT_EQ(gm.getBoard().getSlot(x, 3).getMine(), nullptr);
        EXPECT_TRUE(gm.getBoard().getSlot(x, 3).getShells().empty());
    }

    removeFile(fname);
    std::remove("output_input_missingrow.txt");
}


// 12. tests reading board with extra row
TEST(GameManagerTest, ReadBoardExtraRow) {
    std::string fname = "input_extrarow.txt";
    std::vector<std::string> map = {"1     ", "      ", "2     ", "######", "######"}; 
    createSimpleBoardFile(fname, map, 5, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    EXPECT_NO_THROW(gm.readBoard(fname));
    removeFile(fname);
    std::remove("output_input_extrarow.txt");
}

// 13. tank moves forward to free cell
TEST(GameManagerTest, TankMovesForwardToFreeCell) {
    std::string fname = "input_movefwd.txt";
    std::vector<std::string> map = {" 1    ", "     2", "      ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf;
    MyTankAlgorithmFactory tf;
    GameManager gm(pf, tf);
    gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    gm.executeAction(ActionRequest::MoveForward, t);
    EXPECT_EQ(t->getPosition(), Position(0, 0)); 
    removeFile(fname);
    std::remove("output_input_movefwd.txt");
}

// 14. tank blocked by wall
TEST(GameManagerTest, TankBlockedByWall) {
    std::string fname = "input_wall.txt";
    std::vector<std::string> map = {"#1    ", "      ", "    2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    gm.executeAction(ActionRequest::MoveForward, t);
    EXPECT_EQ(t->getPosition(), Position(1, 0));
    removeFile(fname);
    std::remove("output_input_wall.txt");
}

// 15. tank rotates
TEST(GameManagerTest, TankRotatesCorrectly) {
    std::string fname = "input_rotate.txt";
    std::vector<std::string> map = {" 1    ", "      ", "     2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    Direction orig = t->getDirection();
    gm.executeAction(ActionRequest::RotateLeft45, t);
    EXPECT_NE(t->getDirection(), orig);
    removeFile(fname);
    std::remove("output_input_rotate.txt");
}

// 16. MoveBackward Testing
TEST(GameManagerTest, TankStartMoveBackward) {
    std::string fname = "input_backward.txt";
    std::vector<std::string> map = {" 1    ", "      ", "    2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    gm.executeAction(ActionRequest::MoveBackward, t);
    EXPECT_EQ(t->getBackwardStatus(), 3);
    removeFile(fname);
    std::remove("output_input_backward.txt");
}

// 17. doing and finish MoveBackward
TEST(GameManagerTest, TankFinishMoveBackward) {
    std::string fname = "input_backward2.txt";
    std::vector<std::string> map = {" 1    ", "     2", "      ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    t->setBackwardStatus(2);
    Position orig = t->getPosition();
    gm.executeAction(ActionRequest::MoveBackward, t);
    EXPECT_NE(t->getPosition(), orig); 
    removeFile(fname);
    std::remove("output_input_backward2.txt");
}

// shell fired and added to board
TEST(GameManagerTest, TankShootsAddsShell) {
    std::string fname = "input_shoot.txt";
    std::vector<std::string> map = {" 1    ", "    2", "      ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    auto count_before = gm.getBoard().getShells().size();
    gm.executeAction(ActionRequest::Shoot, t);
    EXPECT_EQ(gm.getBoard().getShells().size(), count_before + 1);
    removeFile(fname);
    std::remove("output_input_shoot.txt");
}

// 19. tank can't shoot when has no ammo
TEST(GameManagerTest, TankCannotShootWithoutShells) {
    std::string fname = "input_shoot2.txt";
    std::vector<std::string> map = {" 1    ", "      ", "    2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    t->setRemainingShells(0);
    auto count_before = gm.getBoard().getShells().size();
    gm.executeAction(ActionRequest::Shoot, t);
    EXPECT_EQ(gm.getBoard().getShells().size(), count_before); // no shell added
    removeFile(fname);
    std::remove("output_input_shoot2.txt");
}

// 20. shell hits tank and destroys it and both are removed
TEST(GameManagerTest, ShellDestroysTank) {
    std::string fname = "input_shelltank.txt";
    std::vector<std::string> map = {" 1 2  ", "     ", "      ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    auto shell = std::make_unique<Shell>(Position(3, 0), Direction(Direction::L), '1');
    gm.getBoard().addShell(std::move(shell));
    gm.checkCollisions();
    EXPECT_EQ(gm.getBoard().getSlot(3, 0).getTank(), nullptr);
    removeFile(fname);
    std::remove("output_input_shelltank.txt");
}

// 21. shell hits wall and reduces its HP to 1
TEST(GameManagerTest, ShellHitsWallReducesHP) {
    std::string fname = "input_shellwall.txt";
    std::vector<std::string> map = {" 1 #  ", "      ", "    2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Wall* w = gm.getBoard().getSlot(3, 0).getWall();
    auto shell = std::make_unique<Shell>(Position(3, 0), Direction(Direction::L), '1');
    gm.getBoard().addShell(std::move(shell));
    int hp1 = w->onHit();
    EXPECT_EQ(hp1, 1);
    removeFile(fname);
    std::remove("output_input_shellwall.txt");
}

// 22. shell collides with wall and destroys it (HP=0 after two hits)
TEST(GameManagerTest, ShellDestroysWall) {
    std::string fname = "input_shellwall2.txt";
    std::vector<std::string> map = {" 1 #  ", "      ", "     2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; 
    GameManager gm(pf, tf); 
    gm.readBoard(fname);

    auto shell1 = std::make_unique<Shell>(Position(3, 0), Direction(Direction::L), '1');
    gm.getBoard().addShell(std::move(shell1));
    gm.checkCollisions();

    auto shell2 = std::make_unique<Shell>(Position(3, 0), Direction(Direction::L), '1');
    gm.getBoard().addShell(std::move(shell2));
    gm.checkCollisions();

    EXPECT_EQ(gm.getBoard().getSlot(3, 0).getWall(), nullptr);
    removeFile(fname);
    std::remove("output_input_shellwall2.txt");
}

// 23. shell collides with another shell and both are removed
TEST(GameManagerTest, ShellHitsShell) {
    std::string fname = "input_shellshell.txt";
    std::vector<std::string> map = {" 1    ", "      ", "     2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    auto shell1 = std::make_unique<Shell>(Position(2, 0), Direction(Direction::L), '1');
    auto shell2 = std::make_unique<Shell>(Position(2, 0), Direction(Direction::L), '2');
    gm.getBoard().addShell(std::move(shell1));
    gm.getBoard().addShell(std::move(shell2));
    gm.checkCollisions();
    EXPECT_EQ(gm.getBoard().getSlot(2, 0).getShells().size(), size_t(0));
    removeFile(fname);
    std::remove("output_input_shellshell.txt");
}

// 24. tank steps on mine and both die
TEST(GameManagerTest, TankStepsOnMine) {
    std::string fname = "input_tankmine.txt";
    std::vector<std::string> map = {" 2@   ", "      ", "    1", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; 
    GameManager gm(pf, tf); 
    gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    gm.executeAction(ActionRequest::MoveForward, t);
    gm.checkCollisions();
    auto& slot = gm.getBoard().getSlot(2, 0);
    EXPECT_TRUE(slot.getAll().empty());
    removeFile(fname);
    std::remove("output_input_tankmine.txt");
}

// 25. Tank collides with another Tank and both die
TEST(GameManagerTest, TankCollidesWithTank) {
    std::string fname = "input_tanktank.txt";
    std::vector<std::string> map = {"1 2   ", "      ", "      2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);

    MyPlayerFactory pf;
    MyTankAlgorithmFactory tf;
    GameManager gm(pf, tf);
    gm.readBoard(fname);

    Tank* t2 = gm.getBoard().getSlot(2, 0).getTank();

    // rotate to be on direction left
    for (int i = 0; i < 4; ++i){
        gm.executeAction(ActionRequest::RotateLeft45, t2);
    }

    // move forward twice
    gm.executeAction(ActionRequest::MoveForward, t2);
    gm.executeAction(ActionRequest::MoveForward, t2);

    gm.checkCollisions();

    // After collision, both tanks should be removed
    CellSlot& slotAfter = gm.getBoard().getSlot(0, 0);
    std::vector<Tank*> tanksAfter;
    for (const auto& obj : slotAfter.getAll()) {
        if (Tank* tank = dynamic_cast<Tank*>(obj.get()))
            tanksAfter.push_back(tank);
    }
    EXPECT_TRUE(tanksAfter.empty());
    removeFile(fname);
    std::remove("output_input_tanktank.txt");
    }

// 26. Game ends at max steps
TEST(GameManagerTest, GameEndsAtMaxSteps) {
    std::string fname = "input_maxsteps.txt";
    std::vector<std::string> map = {"1     ", "      ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6, 10);
    MyPlayerFactory pf; 
    MyTankAlgorithmFactory tf;
    GameManager gm(pf, tf);
    gm.readBoard(fname);

    gm.gameLoop();

    std::ifstream out("output_input_maxsteps.txt"); 
    int numLines = 0;
    std::string line;
    while (std::getline(out, line)) ++numLines;
    out.close();

    EXPECT_EQ(numLines, 11); // 10 turns + 1 line for the end of game message
    removeFile(fname);
    std::remove("output_input_maxsteps.txt");
}



// 27. the game ends after 40 turns without shells
TEST(GameManagerTest, GameEndsAfterNoShells40Turns) {
    std::string fname = "input_noshells.txt";
    std::vector<std::string> map = {"1     ", "      ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf;
    GameManager gm(pf, tf);
    gm.readBoard(fname);
    // reducing shells for each tank to 0
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 6; ++x)
            if (auto t = gm.getBoard().getSlot(x, y).getTank())
                t->setRemainingShells(0);

    gm.gameLoop();

    std::ifstream out("output_input_noshells.txt"); 
    int numLines = 0;
    std::string line;
    while (std::getline(out, line)) ++numLines;
    out.close();
    EXPECT_EQ(numLines, 41); // 40 turns + 1 line for the end of game message
    removeFile(fname);
    std::remove("output_input_noshells.txt");
}



// 28. tanklog updates correctly withhout crashing after running action on tank and the entire game
TEST(GameManagerTest, TankLogUpdatesCorrectly) {
    std::string fname = "input_tanklog.txt";
    std::vector<std::string> map = {"1     ", "      ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf;
    GameManager gm(pf, tf);
    gm.readBoard(fname);
    gm.executeAction(ActionRequest::MoveForward, gm.getBoard().getSlot(0, 0).getTank());
    gm.gameLoop();
    removeFile(fname);
    std::remove("output_input_tanklog.txt");
}

// 29. gameLoop runs till the end without crashing
TEST(GameManagerTest, GameLoopConsistent) {
    std::string fname = "input_gameloop.txt";
    std::vector<std::string> map = {"1     ", "      ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf;
    GameManager gm(pf, tf);
    gm.readBoard(fname);
    gm.gameLoop();
    removeFile(fname);
    std::remove("output_input_gameloop.txt"); 
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}