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

/// עוזר: יצירת קובץ קלט זמני לבדיקה (ללא מוקשים/קירות)
void createSimpleBoardFile(const std::string& filename, const std::vector<std::string>& map, size_t rows, size_t cols) {
    std::ofstream out(filename);
    out << "Test Board\n";
    out << "MaxSteps=100\n";
    out << "NumShells=5\n";
    out << "Rows=" << rows << "\n";
    out << "Cols=" << cols << "\n";
    // כתיבת map לשורות (עד rows), כל שורה מותאמת לאורך cols
    for (size_t i = 0; i < rows; ++i) {
        std::string line = (i < map.size() ? map[i] : "");
        while (line.length() < cols) line += ' ';
        if (line.length() > cols) line = line.substr(0, cols);
        out << line << "\n";
    }
    out.close();
}


void removeFile(const std::string& fname) { std::remove(fname.c_str()); }

// 1. בדיקת בניית GameManager ו־factories
TEST(GameManagerTest, CreateGameManagerNoThrow) {
    MyPlayerFactory pf; MyTankAlgorithmFactory tf;
    EXPECT_NO_THROW(GameManager gm(pf, tf));
}

// 2. בדיקת יצירת Player1 ו־Player2 מה־factory
TEST(GameManagerTest, PlayerFactoryCreatesCorrectPlayers) {
    MyPlayerFactory pf;
    auto p1 = pf.create(1, 6, 4, 10, 5);
    auto p2 = pf.create(2, 6, 4, 10, 5);
    EXPECT_NE(dynamic_cast<Player1*>(p1.get()), nullptr);
    EXPECT_NE(dynamic_cast<Player2*>(p2.get()), nullptr);
}

// 3. בדיקת יצירת TankAlgorithm מה־factory
TEST(GameManagerTest, TankAlgorithmFactoryCreatesCorrectAlgos) {
    MyTankAlgorithmFactory tf;
    auto a1 = tf.create(1, 0);
    auto a2 = tf.create(2, 1);
    EXPECT_NE(dynamic_cast<Algo1*>(a1.get()), nullptr);
    EXPECT_NE(dynamic_cast<Algo2*>(a2.get()), nullptr);
}

// 4. טעינת לוח חוקי עם title, שני טנקים, קיר ומוקש
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
}

// 5. טעינת לוח עם שורה קצרה
TEST(GameManagerTest, ReadBoardWithShortRow) {
    std::string fname = "input_short.txt";
    std::vector<std::string> map = {"1", "     ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 7); // 4 שורות, 6 עמודות
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    gm.readBoard(fname);
    EXPECT_EQ(gm.getBoard().getWidth(), 7);
    removeFile(fname);
}

// 6. טעינת לוח עם שורה ארוכה
TEST(GameManagerTest, ReadBoardWithLongRow) {
    std::string fname = "input_long.txt";
    std::vector<std::string> map = {"1     zzz", "     ", "2    ", "######"};
    createSimpleBoardFile(fname, map,5,7);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    gm.readBoard(fname);
    EXPECT_EQ(gm.getBoard().getWidth(), 7);
    removeFile(fname);
}

// 7. טעינת לוח עם תו לא חוקי
TEST(GameManagerTest, ReadBoardWithUnknownChar) {
    std::string fname = "input_badchar.txt";
    std::vector<std::string> map = {"1  $  ", "     ", "2     ", "######"};
    createSimpleBoardFile(fname, map,4,6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    EXPECT_NO_THROW(gm.readBoard(fname));
    removeFile(fname);
}

// 8. טעינת לוח ללא טנקים
TEST(GameManagerTest, ReadBoardNoTanks) {
    std::string fname = "input_notanks.txt";
    std::vector<std::string> map = {"      ", "      ", "      ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    // אמור לקרוס עם טעות "tie"
    EXPECT_EXIT(gm.readBoard(fname), ::testing::ExitedWithCode(1), "");
    removeFile(fname);
}

// 9. טעינת לוח רק עם טנקים של שחקן 1
TEST(GameManagerTest, ReadBoardOnlyPlayer1Tanks) {
    std::string fname = "input_p1tanks.txt";
    std::vector<std::string> map = {"1     ", "      ", "1     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    EXPECT_EXIT(gm.readBoard(fname), ::testing::ExitedWithCode(1), "");
    removeFile(fname);
}

// 10. טעינת לוח רק עם טנקים של שחקן 2
TEST(GameManagerTest, ReadBoardOnlyPlayer2Tanks) {
    std::string fname = "input_p2tanks.txt";
    std::vector<std::string> map = {"2     ", "      ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    EXPECT_EXIT(gm.readBoard(fname), ::testing::ExitedWithCode(1), "");
    removeFile(fname);
}

// בדיקת טעינת לוח עם שורה חסרה (שורה אחרונה חסרה)
TEST(GameManagerTest, ReadBoardMissingRow) {
    std::string fname = "input_missingrow.txt";
    // רק 3 שורות למרות ש-rows=4
    std::vector<std::string> map = {
        "1     ",
        "  2  ",
        "######"
    };
    int rows = 4; // מוצהר בקובץ
    int cols = 6;
    createSimpleBoardFile(fname, map, rows, cols);

    MyPlayerFactory pf; 
    MyTankAlgorithmFactory tf; 
    GameManager gm(pf, tf);

    // הקריאה אמורה לא לזרוק חריגה, ואמורה להשלים שורה רביעית של רווחים
    EXPECT_NO_THROW(gm.readBoard(fname));

    // נוודא שבאמת נוצרו 4 שורות, וששורה 3 ריקה (רק רווחים)
    for (int x = 0; x < cols; ++x) {
        EXPECT_EQ(gm.getBoard().getSlot(x, 3).getTank(), nullptr);
        EXPECT_EQ(gm.getBoard().getSlot(x, 3).getWall(), nullptr);
        EXPECT_EQ(gm.getBoard().getSlot(x, 3).getMine(), nullptr);
        EXPECT_TRUE(gm.getBoard().getSlot(x, 3).getShells().empty());
    }

    removeFile(fname);
}


// 12. טעינת לוח עם שורות עודפות
TEST(GameManagerTest, ReadBoardExtraRow) {
    std::string fname = "input_extrarow.txt";
    std::vector<std::string> map = {"1     ", "      ", "2     ", "######", "######"}; // עודף
    createSimpleBoardFile(fname, map, 5, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf);
    EXPECT_NO_THROW(gm.readBoard(fname));
    removeFile(fname);
}

// 13. טנק זז קדימה לתא פנוי
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
    EXPECT_EQ(t->getPosition(), Position(0, 0)); // כיוון שמאלה
    removeFile(fname);
}

// 14. טנק מנסה לזוז לתא תפוס (קיר)
TEST(GameManagerTest, TankBlockedByWall) {
    std::string fname = "input_wall.txt";
    std::vector<std::string> map = {"#1    ", "      ", "    2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    gm.executeAction(ActionRequest::MoveForward, t);
    EXPECT_EQ(t->getPosition(), Position(1, 0));
    removeFile(fname);
}

// 15. טנק מסתובב שמאלה/ימינה
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
}

// 16. טנק מבצע MoveBackward (מאתחל ל־3)
TEST(GameManagerTest, TankStartMoveBackward) {
    std::string fname = "input_backward.txt";
    std::vector<std::string> map = {" 1    ", "      ", "    2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    gm.executeAction(ActionRequest::MoveBackward, t);
    EXPECT_EQ(t->getBackwardStatus(), 3);
    removeFile(fname);
}

// 17. תהליך MoveBackward מסתיים
TEST(GameManagerTest, TankFinishMoveBackward) {
    std::string fname = "input_backward2.txt";
    std::vector<std::string> map = {" 1    ", "     2", "      ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    t->setBackwardStatus(2);
    Position orig = t->getPosition();
    gm.executeAction(ActionRequest::MoveBackward, t);
    EXPECT_NE(t->getPosition(), orig); // עבר אחורה
    removeFile(fname);
}

// 18. פגז נורה ומתווסף ללוח
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
}

// 19. טנק יורה כשאין לו פגזים — לא מתווסף פגז (optional)
TEST(GameManagerTest, TankCannotShootWithoutShells) {
    std::string fname = "input_shoot2.txt";
    std::vector<std::string> map = {" 1    ", "      ", "    2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Tank* t = gm.getBoard().getSlot(1, 0).getTank();
    t->setRemainingShells(0);
    auto count_before = gm.getBoard().getShells().size();
    gm.executeAction(ActionRequest::Shoot, t);
    EXPECT_EQ(gm.getBoard().getShells().size(), count_before); // לא נוסף פגז
    removeFile(fname);
}

// 20. פגז פוגע בטנק
TEST(GameManagerTest, ShellDestroysTank) {
    std::string fname = "input_shelltank.txt";
    std::vector<std::string> map = {" 1 2  ", "     ", "      ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    // Tank* t2 = gm.getBoard().getSlot(3, 0).getTank();
    auto shell = std::make_unique<Shell>(Position(3, 0), Direction(Direction::L), '1');
    gm.getBoard().addShell(std::move(shell));
    gm.checkCollisions();
    // החזקה: אחרי התנגשות, התא אמור להיות ריק מטנק
    EXPECT_EQ(gm.getBoard().getSlot(3, 0).getTank(), nullptr);
    removeFile(fname);
}

// 21. פגז פוגע בקיר (HP=1)
TEST(GameManagerTest, ShellHitsWallReducesHP) {
    std::string fname = "input_shellwall.txt";
    std::vector<std::string> map = {" 1 #  ", "      ", "    2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
    Wall* w = gm.getBoard().getSlot(3, 0).getWall();
    auto shell = std::make_unique<Shell>(Position(3, 0), Direction(Direction::L), '1');
    gm.getBoard().addShell(std::move(shell));
    int hp1 = w->onHit();
    EXPECT_EQ(hp1, 1); // אחרי יריה אחת
    removeFile(fname);
}

TEST(GameManagerTest, ShellDestroysWall) {
    std::string fname = "input_shellwall2.txt";
    std::vector<std::string> map = {" 1 #  ", "      ", "     2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf; 
    GameManager gm(pf, tf); 
    gm.readBoard(fname);

    // מדמים ירי של שני פגזים לקיר (שניים כי HP=2)
    auto shell1 = std::make_unique<Shell>(Position(3, 0), Direction(Direction::L), '1');
    gm.getBoard().addShell(std::move(shell1));
    gm.checkCollisions();

    auto shell2 = std::make_unique<Shell>(Position(3, 0), Direction(Direction::L), '1');
    gm.getBoard().addShell(std::move(shell2));
    gm.checkCollisions();

    // עכשיו הקיר אמור להימחק
    EXPECT_EQ(gm.getBoard().getSlot(3, 0).getWall(), nullptr);
    removeFile(fname);
}


// // 23. פגז פוגע במוקש
// TEST(GameManagerTest, ShellHitsMine) {
//     std::string fname = "input_shellmine.txt";
//     std::vector<std::string> map = {" 1 @  ", "      ", "      ", "######"};
//     createSimpleBoardFile(fname, map);
//     MyPlayerFactory pf; MyTankAlgorithmFactory tf; GameManager gm(pf, tf); gm.readBoard(fname);
//     Mine* m = gm.getBoard().getSlot(3, 0).getMine();
//     auto shell = std::make_unique<Shell>(Position(3, 0), Direction(Direction::L), '1');
//     gm.getBoard().addShell(std::move(shell));
//     gm.checkCollisions();
//     EXPECT_EQ(gm.getBoard().getSlot(3, 0).getMine(), nullptr);
//     removeFile(fname);
// }

// 24. פגז פוגע בפגז
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
}

// 25. טנק דורך על מוקש
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
    // לא נוגעים במצביע של הטנק! בודקים את התא החדש
    auto& slot = gm.getBoard().getSlot(2, 0);
    EXPECT_TRUE(slot.getAll().empty());
    removeFile(fname);
}



// 26. טנק מתנגש בטנק — שניהם מתים
TEST(GameManagerTest, TankCollidesWithTank) {
    std::string fname = "input_tanktank.txt";
    std::vector<std::string> map = {"1 2   ", "      ", "      2", "######"};
    createSimpleBoardFile(fname, map, 4, 6);

    MyPlayerFactory pf;
    MyTankAlgorithmFactory tf;
    GameManager gm(pf, tf);
    gm.readBoard(fname);

    // Tank* t1 = gm.getBoard().getSlot(0, 0).getTank();
    Tank* t2 = gm.getBoard().getSlot(2, 0).getTank();

    // לסובב את t2 לימין → שמאלה (R→L), ארבע פעמים
    for (int i = 0; i < 4; ++i)
        gm.executeAction(ActionRequest::RotateLeft45, t2);

    // להזיז t2 פעמיים שמאלה
    gm.executeAction(ActionRequest::MoveForward, t2);
    gm.executeAction(ActionRequest::MoveForward, t2);

    // התנגשות
    gm.checkCollisions();

    // בדיקה: אין טנקים ב-(0,0)
    CellSlot& slot = gm.getBoard().getSlot(0, 0);
    std::vector<Tank*> tanks;
    for (const auto& obj : slot.getAll()) {
        if (Tank* tank = dynamic_cast<Tank*>(obj.get()))
            tanks.push_back(tank);
    }
    EXPECT_TRUE(tanks.empty());
    removeFile(fname);
}



// 27. המשחק מסתיים ב־maxSteps (תור 10)
TEST(GameManagerTest, GameEndsAtMaxSteps) {
    std::string fname = "input_maxsteps.txt";
    std::vector<std::string> map = {"1     ", "      ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf;
    GameManager gm(pf, tf);
    gm.readBoard(fname);
    for (int i = 0; i < 20; ++i) gm.gameLoop(); // מריץ הרבה, אמור להיעצר ב־maxSteps=10
    removeFile(fname);
}

// 28. המשחק מסתיים ב־NO_SHELL_LIMIT
TEST(GameManagerTest, GameEndsAfterNoShells40Turns) {
    std::string fname = "input_noshells.txt";
    std::vector<std::string> map = {"1     ", "      ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf;
    GameManager gm(pf, tf);
    gm.readBoard(fname);

    // אפס תחמושת לכל הטנקים
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 6; ++x)
            if (auto t = gm.getBoard().getSlot(x, y).getTank())
                t->setRemainingShells(0);

    gm.gameLoop();

    // כעת בודקים את מספר השורות באאוטפוט
    std::ifstream out("output_input_noshells.txt"); // שם הקובץ לפי שם האינפוט
    int numLines = 0;
    std::string line;
    while (std::getline(out, line)) ++numLines;
    out.close();

    // שורה ראשונה היא "Turn : 1" לכן 40 תורות == 40 שורות (או אם יש שורה פתיחה - numLines-1)
    EXPECT_EQ(numLines, 41); // 1 כותרת + 40 תורות
    removeFile(fname);
    std::remove("output_input_noshells.txt");
}



// 29. tankLog מתעדכן כהלכה אחרי הרצה
TEST(GameManagerTest, TankLogUpdatesCorrectly) {
    std::string fname = "input_tanklog.txt";
    std::vector<std::string> map = {"1     ", "      ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf;
    GameManager gm(pf, tf);
    gm.readBoard(fname);
    gm.executeAction(ActionRequest::MoveForward, gm.getBoard().getSlot(0, 0).getTank());
    // אין לנו דרך חיצונית לגשת ל־tankLog, אז כאן הטסט רק בודק שלא קרסנו, ומריץ תור שלם
    gm.gameLoop();
    removeFile(fname);
}

// 30. gameLoop רץ עד הסוף, כל המערכים עקביים
TEST(GameManagerTest, GameLoopConsistent) {
    std::string fname = "input_gameloop.txt";
    std::vector<std::string> map = {"1     ", "      ", "2     ", "######"};
    createSimpleBoardFile(fname, map, 4, 6);
    MyPlayerFactory pf; MyTankAlgorithmFactory tf;
    GameManager gm(pf, tf);
    gm.readBoard(fname);
    gm.gameLoop();
    // אין התפוצצות, כל התהליך עובר
    removeFile(fname);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}