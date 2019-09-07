INC_DIR = ./include
INC_MSG = ../message/include
SRC_DIR = src
BUILD_DIR=build

CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pthread
CPPFLAGS = -I$(INC_DIR) -I$(INC_MSG) -fPIC


SHARED = $(BUILD_DIR)/libserver.so
LDFLAGS_SHARED += -shared
LDFLAGS_EXE += -L. $(SHARED) -pthread
LDFLAGS_MSG = -L. ../message/build/libmessage.so -pthread

MAIN_NAME = main
EXE = $(BUILD_DIR)/$(MAIN_NAME)
MAIN = $(EXE:=.o)

SRC_FILES = $(filter-out ./$(SRC_DIR)/$(MAIN_NAME).cpp, $(wildcard ./$(SRC_DIR)/*.cpp))
OBJ = $(patsubst ./$(SRC_DIR)/%.cpp, ./$(BUILD_DIR)/%.o, $(SRC_FILES))


.PHONY:clean all

all: $(BUILD_DIR) $(SHARED) $(EXE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


$(EXE): $(MAIN)
	$(CXX) $(LDFLAGS_EXE) -o $@ $^


$(SHARED): $(OBJ)
	$(CXX) $(LDFLAGS_MSG) $(LDFLAGS_SHARED) -o $@ $^


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<


clean:
	rm -rf build
