CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -fsanitize=address

SRCS = main.cpp \
       GameManager.cpp \
       Board.cpp \
       CellSlot.cpp \
       Position.cpp \
       Direction.cpp \
       Tank.cpp \
       Wall.cpp \
       Shell.cpp \
       Logger.cpp \
       MyTankAlgorithm.cpp \
       MyTankAlgorithmFactory.cpp \
       MySatelliteView.cpp \
       MyBattleInfo.cpp \
       Player1.cpp \
       Player2.cpp \
       MyPlayerFactory.cpp \
       Algo1.cpp \
       Algo2.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = tanks_game

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET) game_output.txt debug_log.txt input_errors.txt
