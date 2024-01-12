.PHONY: test all clean setup

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g3

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++14 -g3
TEST_FLAGS = -lgtest -lgtest_main

BUILD_DIR = build
WORKSPACE = ..
INLCUDE += -I$(WORKSPACE)/CM7/Core/Inc/
INLCUDE += -I$(WORKSPACE)/CM7/Core/Src/
CM7_SRC_DIR = $(WORKSPACE)/CM7/Core/Src

SRC_NAME = uart_state_machine.c
SRC = $(CM7_SRC_DIR)/$(SRC_NAME)
OBJ = $(addprefix $(BUILD_DIR)/,$(SRC_NAME:.c=.o))

TEST_TARGET_NAME = unit_test
TEST_TARGET = $(addprefix $(BUILD_DIR)/,$(TEST_TARGET_NAME))
TEST_SRC = test_uart_state_machine.cpp
TEST_OBJ = $(addprefix $(BUILD_DIR)/,$(TEST_SRC:.cpp=.o))

$(OBJ): $(SRC) setup
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ) $(INLCUDE)

$(TEST_TARGET): $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJ) $(TEST_FLAGS)

$(TEST_OBJ): $(TEST_SRC) setup
	$(CXX) $(CXXFLAGS) -c $(TEST_SRC) -o $(TEST_OBJ) $(INLCUDE)

all: $(TEST_TARGET)

test: all
	./$(TEST_TARGET)

clean:
	rm -r build

setup:
	mkdir -p $(BUILD_DIR)