CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -fPIC -I../common -I.

SRCS = Position.cpp \
       Direction.cpp \
       Logger.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: all clean