CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -fPIC -I../common -I../UserCommon
LDFLAGS = -shared

TARGET = GameManager_318772340_206580102.so

# Note: UserCommon files will be linked as a static library or object files
# For now, assume UserCommon components are available as source
USERCOMMON_DIR = ../UserCommon

# Local GameManager source files
LOCAL_SRCS = GameManager_318772340_206580102.cpp \
             Board.cpp \
             Tank.cpp \
             Shell.cpp \
             Wall.cpp \
             CellSlot.cpp \
             MySatelliteView.cpp

# UserCommon source files that GameManager depends on
USERCOMMON_SRCS = $(USERCOMMON_DIR)/Position.cpp \
                  $(USERCOMMON_DIR)/Direction.cpp \
                  $(USERCOMMON_DIR)/Logger.cpp

ALL_SRCS = $(LOCAL_SRCS) $(USERCOMMON_SRCS)

OBJS = $(ALL_SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ ../Simulator/GameManagerRegistration.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean