CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -O2 -I../common -I../UserCommon
LDFLAGS = -ldl -pthread

TARGET = simulator_318772340_206580102

SRCS = main_comparative_standalone.cpp \
       GameRunner.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) ../GameManager/GameManager_318772340_206580102.o ../GameManager/Board.o ../GameManager/Tank.o ../GameManager/Shell.o ../GameManager/Wall.o ../GameManager/CellSlot.o ../GameManager/MySatelliteView.o GameManagerRegistration.o ../Algorithm/Player_318772340_206580102.o ../Algorithm/TankAlgorithm_318772340_206580102.o ../Algorithm/MyBattleInfo.o PlayerRegistration.o TankAlgorithmRegistration.o ../UserCommon/Position.o ../UserCommon/Direction.o ../UserCommon/Logger.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install: $(TARGET)
	cp $(TARGET) ../

test: $(TARGET)
	@echo "Testing simulator with sample data..."
	@echo "Note: This requires compiled .so files to be available"

test_map_parsing: GameRunner.o
	$(CXX) $(CXXFLAGS) test_map_parsing.cpp GameRunner.o -o test_map_parsing $(LDFLAGS)

test_full_game: GameRunner.o
	$(CXX) $(CXXFLAGS) test_full_game.cpp GameRunner.o -o test_full_game $(LDFLAGS)

test_complete_game: GameRunner.o
	$(CXX) $(CXXFLAGS) test_complete_game.cpp GameRunner.o -o test_complete_game $(LDFLAGS)

test_actual_game: GameRunner.o
	$(CXX) $(CXXFLAGS) test_actual_game.cpp GameRunner.o ../GameManager/GameManager_318772340_206580102.o ../Algorithm/Player_318772340_206580102.o ../Algorithm/TankAlgorithm_318772340_206580102.o -o test_actual_game $(LDFLAGS)

test_run_game: GameRunner.o
	$(CXX) $(CXXFLAGS) test_run_game.cpp GameRunner.o -o test_run_game $(LDFLAGS)

test_single_game_fixed: GameRunner.o
	$(CXX) $(CXXFLAGS) test_single_game_fixed.cpp GameRunner.o -o test_single_game_fixed $(LDFLAGS)

test_single_game_working: GameRunner.o
	$(CXX) $(CXXFLAGS) test_single_game_working.cpp GameRunner.o ../GameManager/GameManager_318772340_206580102.o ../Algorithm/Player_318772340_206580102.o ../Algorithm/TankAlgorithm_318772340_206580102.o -o test_single_game_working $(LDFLAGS)

test_registration_solo: PlayerRegistration.o TankAlgorithmRegistration.o GameManagerRegistration.o
	$(CXX) $(CXXFLAGS) test_registration_solo.cpp PlayerRegistration.o TankAlgorithmRegistration.o GameManagerRegistration.o -o test_registration_solo $(LDFLAGS)

.PHONY: all clean install test test_map_parsing test_full_game test_complete_game test_actual_game test_run_game test_single_game_fixed test_single_game_working test_registration_solo