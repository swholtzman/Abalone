# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wpedantic -march=native -flto
DEBUGFLAGS = -DDEBUG -g

# Directories
SRC_DIR = .
BUILD_DIR = build

# Sources and headers
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/Board.cpp
COMPARE_SRCS = $(SRC_DIR)/compareBoards.cpp
VISUALIZER_SRCS = $(SRC_DIR)/board_visualizer.cpp
PLAY_GAME_SRCS = $(SRC_DIR)/play_game.cpp $(SRC_DIR)/Board.cpp $(SRC_DIR)/TranspositionTable.cpp $(SRC_DIR)/AbaloneAI.cpp

# Targets
TARGET = $(BUILD_DIR)/abalone
COMPARE_TARGET = $(BUILD_DIR)/compareBoards
VISUALIZER_TARGET = $(BUILD_DIR)/board_visualizer
PLAY_GAME_TARGET = $(BUILD_DIR)/play_game

# Default target
all: $(TARGET) $(COMPARE_TARGET) $(VISUALIZER_TARGET) $(PLAY_GAME_TARGET)

# Create build dir if missing
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Main program
$(TARGET): $(SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Debug build
debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

# Comparison tool
$(COMPARE_TARGET): $(COMPARE_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Visualizer
$(VISUALIZER_TARGET): $(VISUALIZER_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

play_game: play_game.cpp Board.cpp TranspositionTable.cpp AbaloneAI.cpp Board.h AbaloneAI.h TranspositionTable.h
	$(CC) $(CFLAGS) play_game.cpp Board.cpp TranspositionTable.cpp AbaloneAI.cpp -o play_game

# Play game
$(PLAY_GAME_TARGET): $(PLAY_GAME_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Visualize input files
visualize:
	./$(VISUALIZER_TARGET) $(word 1, $(MAKECMDGOALS)) $(word 2, $(MAKECMDGOALS))

# Avoid interpreting input args as targets
%:
	@true

# Compare output
diff:
	./$(COMPARE_TARGET) Test2.board boards.txt moves.txt

# Clean everything
clean:
	rm -rf $(BUILD_DIR)