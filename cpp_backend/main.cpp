#include "Board.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <chrono> // Add this for timing functionality


using namespace std;

// Helper: Trim whitespace from the beginning and end of a string.
string trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start])))
        start++;
    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1])))
        end--;
    return s.substr(start, end - start);
}

int main(int argc, char* argv[]) {
    // Check for an input filename argument.
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }
    
    string inputFilename = argv[1];
    
    // Open the original input file.
    ifstream fin(inputFilename);
    if (!fin.is_open()) {
        cerr << "Error: could not open file: " << inputFilename << "\n";
        return 1;
    }
    
    // Read all non-empty lines (after trimming) into a vector.
    vector<string> nonEmptyLines;
    string line;
    while (getline(fin, line)) {
        string trimmed = trim(line);
        if (!trimmed.empty()) {
            nonEmptyLines.push_back(trimmed);
        }
    }
    fin.close();
    
    // Write these non-empty lines to a temporary file.
    ofstream tempOut("temp.input");
    if (!tempOut.is_open()) {
        cerr << "Error: could not create temporary file.\n";
        return 1;
    }
    for (const auto& l : nonEmptyLines) {
        tempOut << l << "\n";
    }
    tempOut.close();
    
    // Now load from the temporary file.
    Board board;
    if (!board.loadFromInputFile("temp.input")) {
        cerr << "Error loading board from temp.input\n";
        return 1;
    }
    
    // Start timing
    auto start = chrono::high_resolution_clock::now();
    
    // Generate moves using board.nextToMove from the file.
    auto moves = board.generateMoves(board.nextToMove);
    
    // End timing
    auto end = chrono::high_resolution_clock::now();
    
    // Calculate duration
    chrono::duration<double, milli> duration = end - start;
    
    // Output the timing information
    cout << "Time to generate all moves: " << duration.count()/1000 << " seconds\n";
    cout << "Number of moves generated: " << moves.size() << "\n";

    size_t pos = inputFilename.find("."); // Find the first dot

    string baseName = (pos != std::string::npos) ? inputFilename.substr(0, pos) : inputFilename;
    
    ofstream movesFile(baseName + ".move");
    ofstream boardsFile(baseName + ".board");
    
    // For each move:
    for (auto& m : moves) {
        // Write the move in document notation.
        string moveNotation = Board::moveToNotation(m, board.nextToMove);
        movesFile << moveNotation << "\n";
        
        // Create a copy and apply the move.
        Board copy = board;
        copy.applyMove(m);
        
        // Write the board state.
        string occupantStr = copy.toBoardString();
        boardsFile << occupantStr << "\n";
    }
    
    return 0;
}