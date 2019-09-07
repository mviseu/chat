INC_DIR = ./include
SRC_DIR = src
BUILD_DIR=build

CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -pthread
CPPFLAGS = -I$(INC_DIR) -fPIC


SHARED = $(BUILD_DIR)/libmessage.so
LDFLAGS_SHARED += -shared



SRC_FILES = $(filter-out ./$(SRC_DIR)/$(MAIN_NAME).cpp, $(wildcard ./$(SRC_DIR)/*.cpp))
OBJ = $(patsubst ./$(SRC_DIR)/%.cpp, ./$(BUILD_DIR)/%.o, $(SRC_FILES))


.PHONY:clean all

all: $(BUILD_DIR) $(SHARED)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)




$(SHARED): $(OBJ)
	$(CXX) $(LDFLAGS_SHARED) -o $@ $^


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<


clean:
	rm -rf build
