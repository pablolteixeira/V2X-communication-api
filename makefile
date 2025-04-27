CC = g++
CFLAGS = -std=c++11 -Wall -pthread
INC = -I./include
LDFLAGS = -pthread

# Directory structure
SRC_DIR = source
INC_DIR = include
BIN_DIR = bin
LOGS_DIR = logs

# Create bin directory if it doesn't exist
$(shell mkdir -p $(BIN_DIR))

# Source files (excluding main.cpp)
SRCS = $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp))
OBJS = $(SRCS:.cpp=.o)

# Default target - build the main program
all: $(BIN_DIR)/main

# Rule to build the main program
$(BIN_DIR)/main: $(SRC_DIR)/main.o $(OBJS)
	$(CC) $(CFLAGS) $(INC) $^ $(LDFLAGS) -o $@

# Rule to compile source files
%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(SRC_DIR)/*.o $(BIN_DIR)/main

# Run the main program
run: $(BIN_DIR)/main
	./$(BIN_DIR)/main

clean_logs:
	rm -f $(LOGS_DIR)/*.log

.PHONY: all clean run