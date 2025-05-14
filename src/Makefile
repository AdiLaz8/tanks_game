CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic 

SRCS = main.cpp GameManager.cpp Board.cpp CellSlot.cpp Position.cpp Direction.cpp Tank.cpp Wall.cpp Shell.cpp Algorithm1.cpp Algorithm2.cpp IAlgorithm.cpp Logger.cpp
OBJS = $(SRCS:.cpp=.o)

TARGET = tanks_game

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET) game_output.txt
