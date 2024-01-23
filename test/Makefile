.PHONY: test all clean setup tmp_test

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g3

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++14 -g3
TEST_FLAGS = -lgtest -lgtest_main

BUILD_DIR = build
WORKSPACE = ..
INLCUDE += -I$(WORKSPACE)/CM7/Core/Inc/
INLCUDE += -I$(WORKSPACE)/CM7/Core/Src/
INLCUDE += -I$(WORKSPACE)/Common/Inc/
CM7_SRC_DIR = $(WORKSPACE)/CM7/Core/Src
COMMON_SRC_DIR = $(WORKSPACE)/Common/Src

CM7_SRCS = $(CM7_SRC_DIR)/uart_state_machine.c
CM7_BUILD_DIR = $(BUILD_DIR)/CM7
COMMON_SRCS = $(COMMON_SRC_DIR)/string_n.c
COMMON_BUILD_DIR = $(BUILD_DIR)/Common
CM7_OBJ = $(addprefix $(CM7_BUILD_DIR)/, $(notdir $(CM7_SRCS:.c=.o)))
COMMON_OBJ = $(addprefix $(COMMON_BUILD_DIR)/, $(notdir $(COMMON_SRCS:.c=.o)))
	  
TEST_EXEC = unit_test
TEST_TARGET = $(addprefix $(BUILD_DIR)/, $(TEST_EXEC))

TESTS = test_main.cpp
TEST_OBJ = $(addprefix $(BUILD_DIR)/, $(TESTS:.cpp=.o))

# $@ is the target, @< is the first prerequisite
$(CM7_OBJ): $(CM7_SRCS) | setup
	@echo "building cm7"
	$(CC) $(CFLAGS) -c $< -o $@ $(INLCUDE)

$(COMMON_OBJ): $(COMMON_SRCS) | setup
	@echo "building common"
	$(CC) $(CFLAGS) -c $< -o $@ $(INLCUDE)

$(TEST_TARGET): $(TEST_OBJ) $(CM7_OBJ) $(COMMON_OBJ)
	@echo "building target"
	$(CXX) $(CXXFLAGS) -o $@ $^ $(TEST_FLAGS)

$(TEST_OBJ): $(TESTS) | setup
	@echo "building testobj"
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INLCUDE)

all: $(TEST_TARGET)

test: all
	./$(TEST_TARGET)

clean:
	rm -r build

# could alternatively use mkdir as a command to make the directories and
#	add the dirs with | as a prerequisite
setup:
	@echo "setting up build folder structure"
	mkdir -p $(BUILD_DIR)
	mkdir -p $(CM7_BUILD_DIR)
	mkdir -p $(COMMON_BUILD_DIR)

tmp_test: $(CM7_SRCS) $(COMMON_SRCS)
	@echo $(CM7_SRCS) $(COMMON_SRCS)
	@echo 
	@echo $(addprefix $(BUILD_DIR)/Common/,$(notdir $(COMMON_SRCS:.c=.o)))
	