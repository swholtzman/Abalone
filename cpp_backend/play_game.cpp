// play_game.cpp
#include "Board.h"
#include "AbaloneAI.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <vector>
#include <chrono>
#include <string>

// Utility function to count marbles of a given side on the board.
int countMarbles(const Board& board, Occupant side) {
    int count = 0;
    for (int i = 0; i < Board::NUM_CELLS; i++) {
        if (board.occupant[i] == side)
            count++;
    }
    return count;
}

int main(int argc, char* argv[]) {
    // Seed the random number generator.
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Default configuration values:
    // winningThreshold: number of opponent marbles pushed off to win (default 4)
    // aiDepth: depth for heuristic AI search (default 4)
    // timeLimitMs: per-move time limit (in milliseconds) for AI search (default 5000)
    // mode: "ai" to use AI for both sides,
    //       "random" for random moves for both sides,
    //       "ai_vs_random" for AI vs random (Black uses AI, White random)
    int winningThreshold = 3;
    int aiDepth = 5;
    int timeLimitMs = 10000;
    std::string mode = "ai_vs_random"; // Options: "ai", "random", or "ai_vs_random"

    // Optional command line arguments override defaults:
    // Usage: ./play_game <winningThreshold> <aiDepth> <timeLimitMs> <mode>
    if (argc >= 2)
        winningThreshold = std::stoi(argv[1]);
    if (argc >= 3)
        aiDepth = std::stoi(argv[2]);
    if (argc >= 4)
        timeLimitMs = std::stoi(argv[3]);
    if (argc >= 5)
        mode = argv[4];

    std::cout << "Starting Abalone game simulation.\n";
    std::cout << "Winning threshold (marbles pushed off): " << winningThreshold << "\n";
    std::cout << "AI search depth: " << aiDepth << ", per-move time limit: " << timeLimitMs << " ms\n";
    std::cout << "Move mode: " << mode
        << " (\"ai\" = both AI, \"random\" = both random, \"ai_vs_random\" = Black AI, White random)\n";

    // Set up the board using the standard starting layout.
    Board board;
    board.initBelgianDaisyLayout();
    // Ensure the first move is by Black.
    board.nextToMove = Occupant::BLACK;

    // Create files for visualization
    std::ofstream initialPositionFile("initial_position.txt");
    std::ofstream movesMadeFile("moves_made.txt");

    initialPositionFile << board.toBoardString() << std::endl;

    int moveCount = 0;
    while (true) {
        std::cout << "\nBoard state: " << board.toBoardString() << "\n";
        int blackMarbles = countMarbles(board, Occupant::BLACK);
        int whiteMarbles = countMarbles(board, Occupant::WHITE);
        std::cout << "Black marbles: " << blackMarbles << " | White marbles: " << whiteMarbles << "\n";

        // Check win condition: if a side has lost enough marbles.
        if ((14 - blackMarbles) >= winningThreshold) {
            std::cout << "White wins! Black has lost " << (14 - blackMarbles) << " marbles.\n";
            break;
        }
        if ((14 - whiteMarbles) >= winningThreshold) {
            std::cout << "Black wins! White has lost " << (14 - whiteMarbles) << " marbles.\n";
            break;
        }

        // Generate legal moves for the current player.
        std::vector<Move> legalMoves = board.generateMoves(board.nextToMove);
        if (legalMoves.empty()) {
            if (board.nextToMove == Occupant::BLACK)
                std::cout << "No legal moves for Black. White wins!\n";
            else
                std::cout << "No legal moves for White. Black wins!\n";
            break;
        }

        Move chosenMove;
        // For Black’s very first move, choose a random legal move.
        if (moveCount == 0 && board.nextToMove == Occupant::BLACK) {
            int randomIndex = std::rand() % legalMoves.size();
            chosenMove = legalMoves[randomIndex];
            std::cout << "Black's first move chosen randomly.\n";
        }
        else {
            // Determine move selection based on the configuration mode.
            if (mode == "ai") {
                // Both sides use AI.
                AbaloneAI aiMove(aiDepth, timeLimitMs);
                auto result = aiMove.findBestMoveIterativeDeepening(board, aiDepth);
                chosenMove = result.first;
                std::cout << (board.nextToMove == Occupant::BLACK ? "Black" : "White")
                    << " (AI) chooses move: "
                    << Board::moveToNotation(chosenMove, board.nextToMove) << "\n";
            }
            else if (mode == "random") {
                // Both sides choose moves randomly.
                int randomIndex = std::rand() % legalMoves.size();
                chosenMove = legalMoves[randomIndex];
                std::cout << (board.nextToMove == Occupant::BLACK ? "Black" : "White")
                    << " (random) chooses move: "
                    << Board::moveToNotation(chosenMove, board.nextToMove) << "\n";
            }
            else if (mode == "ai_vs_random") {
                // Black uses AI; White chooses randomly.
                if (board.nextToMove == Occupant::BLACK) {
                    AbaloneAI aiMove(aiDepth, timeLimitMs);
                    auto result = aiMove.findBestMoveIterativeDeepening(board, aiDepth);
                    chosenMove = result.first;
                    std::cout << "Black (AI) chooses move: "
                        << Board::moveToNotation(chosenMove, board.nextToMove) << "\n";
                }
                else {
                    int randomIndex = std::rand() % legalMoves.size();
                    chosenMove = legalMoves[randomIndex];
                    std::cout << "White (random) chooses move: "
                        << Board::moveToNotation(chosenMove, board.nextToMove) << "\n";
                }
            }
            else {
                std::cerr << "Unknown mode specified. Exiting.\n";
                break;
            }
        }

        // Attempt to apply the chosen move.
        try {
            board.applyMove(chosenMove);

            // Write the new board state to possible moves file
            movesMadeFile << board.toBoardString() << std::endl;
        }
        catch (const std::exception& ex) {
            std::cout << "Error applying move: " << ex.what() << "\n";
            break;
        }

        // Toggle turn.
        board.nextToMove = (board.nextToMove == Occupant::BLACK ? Occupant::WHITE : Occupant::BLACK);
        moveCount++;
    }

    std::cout << "\nGame finished after " << moveCount << " moves.\n";

    // Execute the visualizer (assuming it's compiled as "board_visualizer")
    system(("./board_visualizer initial_position.txt moves_made.txt " + std::string(board.nextToMove == Occupant::BLACK ? "w" : "b")).c_str());

    std::cout << "Board visualization complete. Check visualizer_output.txt for results.\n";

    return 0;
}