#include "AbaloneAI.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // Check for an input filename argument
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> [time_limit_ms] [search_depth]\n";
        return 1;
    }
    
    std::string inputFilename = argv[1];
    
    // Parse optional arguments
    int timeLimit = 5000;  // Default: 5 seconds
    int maxDepth = 4;      // Default: depth 4
    
    if (argc >= 3) {
        timeLimit = std::stoi(argv[2]);
    }
    
    if (argc >= 4) {
        maxDepth = std::stoi(argv[3]);
    }
    
    // Load the board from file
    Board board;
    if (!board.loadFromInputFile(inputFilename)) {
        std::cerr << "Error loading board from " << inputFilename << "\n";
        return 1;
    }
    
    std::cout << "Board loaded successfully. Next to move: " 
              << (board.nextToMove == Occupant::BLACK ? "BLACK" : "WHITE") << "\n";
    
    // Create AI with specified parameters
    AbaloneAI ai(maxDepth, timeLimit);
    
    // Start timing
    auto start = std::chrono::high_resolution_clock::now();
    
    // Find best move with iterative deepening
    std::cout << "Finding best move using iterative deepening (max depth: " << maxDepth << ")...\n";
    auto [bestMove, score] = ai.findBestMoveIterativeDeepening(board, maxDepth);
    
    // End timing
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // Output results
    std::cout << "Search completed in " << elapsed << " ms\n";
    std::cout << "Best move: " << Board::moveToNotation(bestMove, board.nextToMove) << "\n";
    std::cout << "Evaluation score: " << score << " (positive favors BLACK, negative favors WHITE)\n";
    
    // Apply the move to see the resulting board
    Board resultBoard = board;
    resultBoard.applyMove(bestMove);
    std::cout << "Resulting board state: " << resultBoard.toBoardString() << "\n";
    
    return 0;
}