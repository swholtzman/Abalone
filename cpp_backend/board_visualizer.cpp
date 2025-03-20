#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>

// Structure to represent a marble
struct Marble {
    char color; // 'b' for black, 'w' for white, ' ' for empty
};

// Structure to represent a coordinate on the board
struct Coordinate {
    char column;
    int row;
    
    Coordinate(char c, int r) : column(c), row(r) {}
    
    bool operator<(const Coordinate& other) const {
        if (column != other.column) {
            return column < other.column;
        }
        return row < other.row;
    }
};

// Function to parse the game state
std::map<Coordinate, Marble> parseGameState(const std::string& gameState) {
    std::map<Coordinate, Marble> board;
    std::stringstream ss(gameState);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        if (token.length() >= 3) {
            char column = token[0];
            int row = token[1] - '0';
            char color = token[2];
            board[Coordinate(column, row)] = {color};
        }
    }
    
    return board;
}

// Function to display the Abalone board to a file
void displayBoardToFile(const std::map<Coordinate, Marble>& board, std::ofstream& outputFile) {
    // Define valid positions based on the Abalone board layout
    std::vector<std::vector<std::string>> validPositions = {
        {"I5", "I6", "I7", "I8", "I9"},                   // Row 0
        {"H4", "H5", "H6", "H7", "H8", "H9"},             // Row 1
        {"G3", "G4", "G5", "G6", "G7", "G8", "G9"},       // Row 2
        {"F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9"}, // Row 3
        {"E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9"}, // Row 4
        {"D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"}, // Row 5
        {"C1", "C2", "C3", "C4", "C5", "C6", "C7"},       // Row 6
        {"B1", "B2", "B3", "B4", "B5", "B6"},             // Row 7
        {"A1", "A2", "A3", "A4", "A5"}                    // Row 8
    };
    
    // Print the board to file
    int maxRowLength = validPositions[4].size();  // The longest row (E row)
    const int spaceWidth = 3; // Width of each position including parentheses
    
    for (size_t r = 0; r < validPositions.size(); r++) {
        // Calculate leading spaces based on the difference in elements from longest row
        int spaceDifference = maxRowLength - validPositions[r].size();
        int leadingSpaces = spaceDifference * spaceWidth / 2;
        outputFile << std::string(leadingSpaces, ' ');
        
        for (const auto& pos : validPositions[r]) {
            char column = pos[0];
            int row = pos[1] - '0';
            
            Coordinate coord(column, row);
            
            // Check if there's a marble at this position
            auto it = board.find(coord);
            
            outputFile << "(";
            if (it != board.end()) {
                if (it->second.color == 'b') {
                    outputFile << "B"; // Black marble
                } else if (it->second.color == 'w') {
                    outputFile << "W"; // White marble
                }
            } else {
                outputFile << "·"; // Empty position
            }
            outputFile << ")";
        }
        
        outputFile << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <initial_position_file> <possible_moves_file>" << std::endl;
        return 1;
    }
    
    // Open initial position file
    std::ifstream initialPositionFile(argv[1]);
    if (!initialPositionFile) {
        std::cerr << "Error: Could not open initial position file " << argv[1] << std::endl;
        return 1;
    }
    
    // Open possible moves file
    std::ifstream possibleMovesFile(argv[2]);
    if (!possibleMovesFile) {
        std::cerr << "Error: Could not open possible moves file " << argv[2] << std::endl;
        return 1;
    }
    
    // Always write to "output.txt"
    std::ofstream outputFile("visualizer_output.txt");
    if (!outputFile) {
        std::cerr << "Error: Could not open output file output.txt" << std::endl;
        return 1;
    }
    
    // Process initial position
    // std::string firstLine, initialGameState;
    
    // Process initial position
    std::string initialGameState;
    
    // Read the first line (next to play)
    // std::getline(initialPositionFile, firstLine);
    
    // Read the second line (initial game state)
    if (std::getline(initialPositionFile, initialGameState)) {
        std::map<Coordinate, Marble> initialBoard = parseGameState(initialGameState);
        
        // Write initial position to the output file
        outputFile << "Initial Abalone Game Board:" << std::endl << std::endl;
        displayBoardToFile(initialBoard, outputFile);
        outputFile << "\nInitial Game State: " << initialGameState << std::endl;
        // outputFile << "Current Player: " << (firstLine == "b" ? "Black" : "White") << std::endl;
        outputFile << "\n--------------------------------------------------\n\n";
    } else {
        std::cerr << "Error: Initial position file does not have a game state line" << std::endl;
        return 1;
    }
    
    // Process possible moves
    // outputFile << "Possible Moves:" << std::endl << std::endl;
    
    std::string moveGameState;
    int moveCount = 1;
    
    while (std::getline(possibleMovesFile, moveGameState)) {
        if (!moveGameState.empty()) {
            std::map<Coordinate, Marble> moveBoard = parseGameState(moveGameState);
            
            // Write this possible move to the output file
            outputFile << "Move #" << moveCount << ":" << std::endl << std::endl;
            displayBoardToFile(moveBoard, outputFile);
            outputFile << "\nGame State: " << moveGameState << std::endl;
            outputFile << "\n--------------------------------------------------\n\n";
            
            moveCount++;
        }
    }
    
    std::cout << "Board and " << (moveCount - 1) << " possible moves successfully written to output.txt" << std::endl;
    
    return 0;
}