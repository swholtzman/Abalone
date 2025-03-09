CC = g++
CFLAGS = -std=c++17
DEBUGFLAGS = -DDEBUG

# Source files
SRCS = main.cpp Board.cpp
COMPARE_SRCS = compareBoards.cpp

# Output binaries
TARGET = abalone
COMPARE_TARGET = compareBoards

# Default target
all: $(TARGET) $(COMPARE_TARGET)

# Compile the main program
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Compile the main program with debug messages
debug:
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(SRCS) -o $(TARGET)

# Compile the comparison tool
$(COMPARE_TARGET): $(COMPARE_SRCS)
	$(CC) $(CFLAGS) $(COMPARE_SRCS) -o $(COMPARE_TARGET)

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
	rm -f $(TARGET) $(COMPARE_TARGET)
