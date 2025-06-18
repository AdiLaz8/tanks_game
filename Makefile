CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -fsanitize=address -I./src -I./common

SRCS = main.cpp \
       src/GameManager.cpp \
       src/Board.cpp \
       src/CellSlot.cpp \
       src/Position.cpp \
       src/Direction.cpp \
       src/Tank.cpp \
       src/Wall.cpp \
       src/Shell.cpp \
       src/Logger.cpp \
       src/MyTankAlgorithm.cpp \
       src/MyTankAlgorithmFactory.cpp \
       src/MySatelliteView.cpp \
       src/MyBattleInfo.cpp \
       src/Player1.cpp \
       src/Player2.cpp \
       src/MyPlayerFactory.cpp \
       src/Algo1.cpp \
       src/Algo2.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = tanks_game

TEST_OBJS = $(filter-out main.o, $(OBJS))

GTEST_INC = -I$(HOME)/Downloads/googletest/googletest/include
GTEST_LIB = $(HOME)/Downloads/googletest/build/lib/libgtest.a $(HOME)/Downloads/googletest/build/lib/libgtest_main.a

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test_game: src/test_game.cpp $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(GTEST_INC) -o $@ src/test_game.cpp $(TEST_OBJS) $(GTEST_LIB) -lpthread

test: test_game
	./test_game

clean:
	rm -f $(TARGET) test_game game_output.txt debug_log.txt input_errors.txt
	find . -name "*.o" -delete
