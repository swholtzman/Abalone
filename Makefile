CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic

# Executables
GENERATOR = abalone_generator

# Source files
GENERATOR_SRC = abalone_generator.cpp

all: $(GENERATOR)

$(GENERATOR): $(GENERATOR_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(GENERATOR)

test: $(GENERATOR)
	./$(GENERATOR) Test1.input
	./$(GENERATOR) Test2.input
	@echo "Test1.move and Test1.board created."
	@echo "Test2.move and Test2.board created."
