CC = g++
CFLAGS = -std=c++17
DEBUGFLAGS = -DDEBUG

# Source files
SRCS = main.cpp Board.cpp
COMPARE_SRCS = compareBoards.cpp
VISUALIZER_SRC = board_visualizer.cpp

# Output binaries
TARGET = abalone
COMPARE_TARGET = compareBoards
VISUALIZER_TARGET = board_visualizer

# Default target
all: $(TARGET) $(COMPARE_TARGET) $(VISUALIZER_TARGET)

# Compile the main program
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Compile the main program with debug messages
debug:
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(SRCS) -o $(TARGET)

# Compile the comparison tool
$(COMPARE_TARGET): $(COMPARE_SRCS)
	$(CC) $(CFLAGS) $(COMPARE_SRCS) -o $(COMPARE_TARGET)

# Compile the visualizer program
$(VISUALIZER_TARGET): $(VISUALIZER_SRC)
	$(CC) $(CFLAGS) $(VISUALIZER_SRC) -o $(VISUALIZER_TARGET)

# Run the visualizer with custom input files
visualize:
	./$(VISUALIZER_TARGET) $(word 1,$(MAKECMDGOALS)) $(word 2,$(MAKECMDGOALS))

# Run play_game: g++ -std=c++17 play_game.cpp Board.cpp TranspositionTable.cpp AbaloneAI.cpp -o play_game
#play_game:
#	$(CC) $(CFLAGS) play_game.cpp Board.cpp TranspositionTable.cpp AbaloneAI.cpp -o play_game
play_game: play_game.cpp Board.cpp TranspositionTable.cpp AbaloneAI.cpp Board.h AbaloneAI.h TranspositionTable.h
	$(CC) $(CFLAGS) play_game.cpp Board.cpp TranspositionTable.cpp AbaloneAI.cpp -o play_game

# Prevent Make from treating the arguments as targets
%:
	@:

# Run the main program with a specified input file
# run:
# 	./$(TARGET) $(ARGS)

# # Run with debug messages and a specified input file
# run-debug:
# 	./$(TARGET) $(ARGS)

# Compare output
diff:
	./$(COMPARE_TARGET) Test2.board boards.txt moves.txt

# Clean generated files
clean:
	rm -f $(TARGET) $(COMPARE_TARGET) $(VISUALIZER_TARGET) play_game