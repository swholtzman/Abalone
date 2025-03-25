<<<<<<< HEAD
#ifndef ABALONE_AI_H
#define ABALONE_AI_H

#include "Board.h"
#include "TranspositionTable.h"
#include <chrono>
#include <utility>

class AbaloneAI {
private:
    // Maximum search depth
    int maxDepth;
    // Number of positions evaluated
    int nodesEvaluated;
    // Time limit for search in milliseconds
    int timeLimit;
    // Start time of search
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    // Indicates if search was terminated due to time limit
    bool timeoutOccurred;

    TranspositionTable transpositionTable;
    
    // Piece value
    static const int MARBLE_VALUE = 100;

    static int PRUNES_OCCURED;
    /**
     * Evaluates the current board position from BLACK's perspective.
     * Higher scores are better for BLACK, lower scores for WHITE.
     */
    int evaluatePosition(const Board& board);
    
    /**
     * Calculates group cohesion for the given side.
     */
    int calculateCohesion(const Board& board, Occupant side);
    
    /**
     * Calculates how many marbles are in edge positions (risk of being pushed off).
     */
    int calculateEdgeDanger(const Board& board, Occupant side);
    
    /**
     * Checks if the time limit has been exceeded.
     */
    bool isTimeUp();

    /**
     * The minimax algorithm with alpha-beta pruning.
     */
    int minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer);

public:
    // Default parameters are specified only here.
    AbaloneAI(int depth = 4, int timeLimitMs = 5000, size_t ttSizeInMB = 64);
    
    /**
     * Finds the best move for the given board position.
     * Returns the best move and its evaluation score.
     */
    std::pair<Move, int> findBestMove(Board& board);
    
    /**
     * Iterative deepening search.
     * Default max search depth is 10.
     */
    std::pair<Move, int> findBestMoveIterativeDeepening(Board& board, int maxSearchDepth = 10);
};

#endif // ABALONE_AI_H
=======
#ifndef ABALONE_AI_H
#define ABALONE_AI_H

#include "Board.h"
#include "TranspositionTable.h"
#include <chrono>
#include <utility>
#include <vector>
#include <array>

class AbaloneAI {
private:
    // Maximum search depth
    int maxDepth;
    // Number of positions evaluated
    int nodesEvaluated;
    // Time limit for search in milliseconds
    int timeLimit;
    // Start time of search
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    // Indicates if search was terminated due to time limit
    bool timeoutOccurred;

    TranspositionTable transpositionTable;

    // Piece value
    static const int MARBLE_VALUE = 100;

    // Count the number of times pruning occurs
    int pruningCount = 0;

    // Killer move heuristic - stores two killer moves per depth
    static constexpr int MAX_KILLER_MOVES = 2;
    std::vector<std::array<Move, MAX_KILLER_MOVES>> killerMoves;

    // Helper method to update killer moves
    void updateKillerMove(const Move& move, int depth);

    // Helper function to check if a move is a killer move
    bool isKillerMove(const Move& move, int depth) const;

    // Evaluation functions
    int evaluatePosition(const Board& board);
    int evaluateMove(const Board& board, const Move& move, Occupant side);

    // Additional evaluation functions (added based on the implementation)
    int calculatePositionalAdvantage(const Board& board, Occupant side);
    int calculateSumitoAdvantages(const Board& board, Occupant side);
    int calculateMobility(const Board& board, Occupant side);
    int calculateFormations(const Board& board, Occupant side);

    // Board evaluation helper functions
    int calculateCohesion(const Board& board, Occupant side);
    int calculateEdgeDanger(const Board& board, Occupant side);
    int calculateThreatPotential(const Board& board, Occupant side);

    // Checks if the time limit has been exceeded.
    bool isTimeUp();

    // The minimax algorithm with alpha-beta pruning.
    int minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer);

    // Order moves based on evaluation and transposition table move
    void orderMoves(std::vector<Move>& moves, const Board& board, Occupant side, const Move& ttMove);

public:
    // Constructor with default parameters.
    AbaloneAI(int depth = 4, int timeLimitMs = 5000, size_t ttSizeInMB = 64);

    // Finds the best move for the given board position.
    std::pair<Move, int> findBestMove(Board& board);

    // Iterative deepening search.
    std::pair<Move, int> findBestMoveIterativeDeepening(Board& board, int maxSearchDepth = 10);
};

#endif // ABALONE_AI_H
>>>>>>> logan-heuristic-v2
