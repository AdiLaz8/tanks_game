CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -fPIC -I../common -I../UserCommon
LDFLAGS = -shared -Wl,-soname,$(TARGET)

TARGET = Algorithm_318772340_206580102.so


USERCOMMON_DIR = ../UserCommon

# Local Algorithm source files
LOCAL_SRCS = Player_318772340_206580102.cpp \
             TankAlgorithm_318772340_206580102.cpp \
             MyBattleInfo.cpp

# UserCommon source files that Algorithm depends on
USERCOMMON_SRCS = $(USERCOMMON_DIR)/Position.cpp \
                  $(USERCOMMON_DIR)/Direction.cpp

ALL_SRCS = $(LOCAL_SRCS) $(USERCOMMON_SRCS)

OBJS = $(ALL_SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean