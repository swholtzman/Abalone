CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -O2
# DEBUGFLAGS = -DDEBUG

# Source files
SRCS = main.cpp Board.cpp
AI_SRCS = AbaloneAI.cpp AIExample.cpp Board.cpp
COMPARE_SRCS = compareBoards.cpp
VISUALIZER_SRC = board_visualizer.cpp

# Output binaries
TARGET = abalone
AI_TARGET = abalone_ai
COMPARE_TARGET = compareBoards
VISUALIZER_TARGET = board_visualizer

# Default target
all: $(TARGET) $(AI_TARGET) $(COMPARE_TARGET) $(VISUALIZER_TARGET)

# Compile the main program
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Compile the AI program
$(AI_TARGET): $(AI_SRCS)
	$(CC) $(CFLAGS) $(AI_SRCS) -o $(AI_TARGET)

# Compile the main program with debug messages
debug:
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(SRCS) -o $(TARGET)

# Compile the AI program with debug messages
ai-debug:
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(AI_SRCS) -o $(AI_TARGET)

# Compile the comparison tool
$(COMPARE_TARGET): $(COMPARE_SRCS)
	$(CC) $(CFLAGS) $(COMPARE_SRCS) -o $(COMPARE_TARGET)

# Compile the visualizer program
$(VISUALIZER_TARGET): $(VISUALIZER_SRC)
	$(CC) $(CFLAGS) $(VISUALIZER_SRC) -o $(VISUALIZER_TARGET)

# Run the visualizer with custom input files
visualize:
	./$(VISUALIZER_TARGET) $(word 1,$(MAKECMDGOALS)) $(word 2,$(MAKECMDGOALS))

# Run the AI program with a specified input file
run-ai:
	./$(AI_TARGET) $(word 1,$(MAKECMDGOALS))

# Run the AI program with specific time limit and search depth
run-ai-advanced:
	./$(AI_TARGET) $(word 1,$(MAKECMDGOALS)) $(word 2,$(MAKECMDGOALS)) $(word 3,$(MAKECMDGOALS))

# Prevent Make from treating the arguments as targets
%:
	@:

# Compare output
diff:
	./$(COMPARE_TARGET) Test2.board boards.txt moves.txt

# Clean generated files
clean:
	rm -f $(TARGET) $(AI_TARGET) $(COMPARE_TARGET) $(VISUALIZER_TARGET) *.o