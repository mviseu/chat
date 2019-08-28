INC_DIR = ./include
SRC_DIR = src
BUILD_DIR=build

CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pthread
CPPFLAGS = -I$(INC_DIR) -fPIC


SHARED = $(BUILD_DIR)/libclient.so
LDFLAGS_SHARED += -shared
LDFLAGS_EXE += -L. $(SHARED) -pthread

MAIN_NAME = main
EXE = $(BUILD_DIR)/$(MAIN_NAME)
MAIN = $(EXE:=.o)

SRC_FILES = $(filter-out ./$(SRC_DIR)/$(MAIN_NAME).cpp, $(wildcard ./$(SRC_DIR)/*.cpp))
OBJ = $(patsubst ./$(SRC_DIR)/%.cpp, ./$(BUILD_DIR)/%.o, $(SRC_FILES))



all: $(SHARED) $(EXE)


$(EXE): $(MAIN)
	$(CXX) $(LDFLAGS_EXE) -o $@ $^


$(SHARED): $(OBJ)
	$(CXX) $(LDFLAGS_SHARED) -o $@ $^


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<


.PHONY:clean


clean:
	rm -rf build/*
