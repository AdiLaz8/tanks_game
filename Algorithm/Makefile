CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -fPIC -I../common -I../UserCommon
LDFLAGS = -shared

TARGET = Algorithm_318772340_206580102.so

# Note: UserCommon files will be linked as a static library or object files
# For now, assume UserCommon components are available as source
USERCOMMON_DIR = ../UserCommon

# Local Algorithm source files
LOCAL_SRCS = Player_318772340_206580102.cpp \
             TankAlgorithm_318772340_206580102.cpp \
             MyBattleInfo.cpp \
             PlayerRegistration.cpp \
             TankAlgorithmRegistration.cpp

# UserCommon source files that Algorithm depends on
USERCOMMON_SRCS = $(USERCOMMON_DIR)/Position.cpp \
                  $(USERCOMMON_DIR)/Direction.cpp \
                  $(USERCOMMON_DIR)/Logger.cpp

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