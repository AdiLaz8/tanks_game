# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

# Source files
SRCS = main.cpp \
       Board.cpp \
       Position.cpp \
       Direction.cpp \
       Tank.cpp \
       Wall.cpp \
       Shell.cpp \
       CellSlot.cpp


# Object files (כל קובץ cpp יומר ל־o)
OBJS = $(SRCS:.cpp=.o)

# Target binary name
TARGET = tanks_game

# Default target
all: $(TARGET)

# Build the target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
