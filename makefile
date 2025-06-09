CC = g++
# -fsanitize=address
CFLAGS = -std=c++11 -Wall  -pthread
INC = -I./include
LDFLAGS = -pthread

# Directory structure
SRC_DIR = source
INC_DIR = include
BIN_DIR = bin
LOGS_DIR = logs
TEST_DIR = tests

# Create bin directory if it doesn't exist
$(shell mkdir -p $(BIN_DIR))
$(shell mkdir -p $(TEST_DIR))

# Source files (excluding main.cpp)
SRCS = $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp))
OBJS = $(SRCS:.cpp=.o)

# Test files
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
TEST_BINS = $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/test_%,$(TEST_SRCS))

# Default target - build the main program
#all: clean clean_logs run_tests run
all: clean clean_logs run_tests run

clean_all: clean clean_logs

# Rule to build the main program
$(BIN_DIR)/main: $(SRC_DIR)/main.o $(OBJS)
	$(CC) $(CFLAGS) $(INC) $^ $(LDFLAGS) -o $@

# Rule to compile source files
%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

# Rule to build test executables
$(BIN_DIR)/test_%: $(TEST_DIR)/%.cpp $(OBJS)
	$(CC) $(CFLAGS) $(INC) $^ $(LDFLAGS) -o $@

# Build all tests
tests: $(TEST_BINS)

# Run all tests
run_tests: tests
	@echo "Running all tests..."
	@for test in $(TEST_BINS); do \
		echo "\n--- Running $$test ---"; \
		$$test; \
		if [ $$? -eq 0 ]; then \
			echo "    Test PASSED"; \
		else \
			echo "    Test FAILED"; \
		fi; \
	done

# Run a specific test (usage: make run_test TEST=test_name)
run_test: $(BIN_DIR)/test_$(TEST)
	./$(BIN_DIR)/test_$(TEST)

# Clean build artifacts
clean:
	rm -f $(SRC_DIR)/*.o $(BIN_DIR)/*

# Clean test artifacts
clean_tests:
	rm -f $(TEST_DIR)/*.o

# Run the main program
run: $(BIN_DIR)/main
	./$(BIN_DIR)/main

clean_logs:
	rm -f $(LOGS_DIR)/*.log
	rm -f *.txt

.PHONY: all clean run