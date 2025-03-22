#include "AbaloneAI.h"
#include "Board.h"
#include <limits>
#include <algorithm>
#include <chrono>
#include <iostream>

// Evaluate the board position from BLACK's perspective
int AbaloneAI::evaluatePosition(const Board& board) {
    nodesEvaluated++;
    
    // Count marbles for each side
    int blackMarbles = 0;
    int whiteMarbles = 0;
    for (int i = 0; i < Board::NUM_CELLS; i++) {
        if (board.occupant[i] == Occupant::BLACK)
            blackMarbles++;
        else if (board.occupant[i] == Occupant::WHITE)
            whiteMarbles++;
    }
    
    // Basic score: difference in marble counts
    int score = (blackMarbles - whiteMarbles) * MARBLE_VALUE;
    
    // Component 1: Center control bonus
    int blackCenterControl = 0;
    int whiteCenterControl = 0;
    std::vector<int> centerCells = {
        Board::notationToIndex("E5"),
        Board::notationToIndex("D5"),
        Board::notationToIndex("F5"),
        Board::notationToIndex("E4"),
        Board::notationToIndex("E6")
    };
    for (int idx : centerCells) {
        if (idx >= 0) {
            if (board.occupant[idx] == Occupant::BLACK)
                blackCenterControl++;
            else if (board.occupant[idx] == Occupant::WHITE)
                whiteCenterControl++;
        }
    }
    score += (blackCenterControl - whiteCenterControl) * 10;
    
    // Component 2: Group cohesion bonus
    int blackCohesion = calculateCohesion(board, Occupant::BLACK);
    int whiteCohesion = calculateCohesion(board, Occupant::WHITE);
    score += (blackCohesion - whiteCohesion) * 5;
    
    // Component 3: Edge danger penalty
    int blackEdgeDanger = calculateEdgeDanger(board, Occupant::BLACK);
    int whiteEdgeDanger = calculateEdgeDanger(board, Occupant::WHITE);
    score -= (blackEdgeDanger - whiteEdgeDanger) * 15;

    // // Component 4: Mobility bonus
    // int blackMobility = calculateMobility(board, Occupant::BLACK);
    // int whiteMobility = calculateMobility(board, Occupant::WHITE);
    // score += (blackMobility - whiteMobility) * 3;
    
    // Component 5: Threat potential bonus
    int blackThreats = calculateThreatPotential(board, Occupant::BLACK);
    int whiteThreats = calculateThreatPotential(board, Occupant::WHITE);
    score += (blackThreats - whiteThreats) * 10;
    
    return score;
}

// Evaluate a move quickly for ordering purposes
int AbaloneAI::evaluateMove(const Board& board, const Move& move, Occupant side) {
    int score = 0;
    
    // Prioritize captures
    if (move.pushCount > 0) {
        score += 1000 * move.pushCount;  // Higher score for more captures
    }
    
    // Calculate center of the board (approximately E5 in standard notation)
    int centerIdx = Board::notationToIndex("E5");
    
    // Prioritize moves towards the center
    // We'll use the end positions of the marbles after the move
    Board tempBoard = board;
    tempBoard.applyMove(move);
    
    // Check if the move improves centralization
    double beforeCentralization = 0;
    double afterCentralization = 0;
    
    for (int idx : move.marbleIndices) {
        if (idx >= 0) {
            // Before position - distance from center
            auto beforeCoord = board.s_indexToCoord[idx];
            auto centerCoord = board.s_indexToCoord[centerIdx];
            int distBefore = std::abs(beforeCoord.first - centerCoord.first) + 
                            std::abs(beforeCoord.second - centerCoord.second);
            beforeCentralization += distBefore;
            
            // Calculate where this marble ended up (approximately)
            // This is a simplification, as the exact end position depends on the move mechanics
            int endIdx = board.neighbors[idx][move.direction];
            if (endIdx >= 0) {
                auto afterCoord = board.s_indexToCoord[endIdx];
                int distAfter = std::abs(afterCoord.first - centerCoord.first) + 
                               std::abs(afterCoord.second - centerCoord.second);
                afterCentralization += distAfter;
            }
        }
    }
    
    // Add points if the move improves centralization (lower distance is better)
    if (afterCentralization < beforeCentralization) {
        score += (beforeCentralization - afterCentralization) * 10;
    }
    
    // Prioritize group-forming moves
    int beforeCohesion = calculateCohesion(board, side);
    int afterCohesion = calculateCohesion(tempBoard, side);
    score += (afterCohesion - beforeCohesion) * 5;
    
    // Penalize moves that put marbles in danger
    int beforeDanger = calculateEdgeDanger(board, side);
    int afterDanger = calculateEdgeDanger(tempBoard, side);
    score -= (afterDanger - beforeDanger) * 15;

    // Bonus for pushing opponent marbles off the edge
    if (move.pushCount > 0) {
        score += 50 * move.pushCount;
    }
    
    // Bonus for inline moves (usually more powerful)
    if (move.isInline) {
        score += 20;
    }
    
    // Bonus for moves that go away from the edge if we're already in danger
    if (beforeDanger > 0) {
        score += (beforeDanger - afterDanger) * 20;
    }
    
    return score;
}

int AbaloneAI::calculateCohesion(const Board& board, Occupant side) {
    int cohesion = 0;
    for (int i = 0; i < Board::NUM_CELLS; i++) {
        if (board.occupant[i] == side) {
            for (int d = 0; d < Board::NUM_DIRECTIONS; d++) {
                int neighbor = board.neighbors[i][d];
                if (neighbor >= 0 && board.occupant[neighbor] == side)
                    cohesion++;
            }
        }
    }
    return cohesion;
}

int AbaloneAI::calculateEdgeDanger(const Board& board, Occupant side) {
    int edgeCount = 0;
    for (int i = 0; i < Board::NUM_CELLS; i++) {
        if (board.occupant[i] == side) {
            bool onEdge = false;
            for (int d = 0; d < Board::NUM_DIRECTIONS; d++) {
                if (board.neighbors[i][d] < 0) {  // neighbor off-board
                    onEdge = true;
                    break;
                }
            }
            if (onEdge)
                edgeCount++;
        }
    }
    return edgeCount;
}

int AbaloneAI::calculateThreatPotential(const Board& board, Occupant side) {
    int threatScore = 0;
    
    // Check for potential threats in each direction
    for (int i = 0; i < Board::NUM_CELLS; i++) {
        if (board.occupant[i] == side) {
            for (int d = 0; d < Board::NUM_DIRECTIONS; d++) {
                int neighbor = board.neighbors[i][d];
                if (neighbor >= 0 && board.occupant[neighbor] == Occupant::EMPTY) {
                    // Check if the next cell is an opponent
                    int nextNeighbor = board.neighbors[neighbor][d];
                    if (nextNeighbor >= 0 && board.occupant[nextNeighbor] != side) {
                        threatScore++;
                    }
                }
            }
        }
    }
    
    return threatScore;
}

bool AbaloneAI::isTimeUp() {
    if (timeLimit <= 0)
        return false;
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
    return elapsed >= timeLimit;
}

// Helper function to sort moves based on their evaluation
void AbaloneAI::orderMoves(std::vector<Move>& moves, const Board& board, Occupant side, const Move& ttMove) {
    // Define a struct to hold moves and their scores
    struct ScoredMove {
        Move move;
        int score;
        
        ScoredMove(const Move& m, int s) : move(m), score(s) {}
        
        // For sorting in descending order (highest score first)
        bool operator<(const ScoredMove& other) const {
            return score > other.score;
        }
    };
    
    std::vector<ScoredMove> scoredMoves;
    
    // First, check if ttMove is in the list and give it highest priority
    bool ttMoveFound = false;
    for (const Move& move : moves) {
        if (move == ttMove) {
            // Give TT move highest priority
            scoredMoves.push_back(ScoredMove(move, std::numeric_limits<int>::max()));
            ttMoveFound = true;
        } else {
            // Score other moves
            int moveScore = evaluateMove(board, move, side);
            scoredMoves.push_back(ScoredMove(move, moveScore));
        }
    }
    
    // Sort moves by score
    std::sort(scoredMoves.begin(), scoredMoves.end());
    
    // Update the original vector with sorted moves
    for (size_t i = 0; i < moves.size(); i++) {
        moves[i] = scoredMoves[i].move;
    }
}

int AbaloneAI::minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer) {
    if (isTimeUp()) {
        timeoutOccurred = true;
        return evaluatePosition(board);
    }
    if (depth == 0)
        return evaluatePosition(board);

    // Check transposition table first
    int origAlpha = alpha;
    int origBeta = beta;
    Move bestMove;
    int score;
    MoveType moveType;
    
    // Check if the current board position is already stored in the transposition table
    if (transpositionTable.probeEntry(board, depth, score, moveType, bestMove)) {
        // TT hit - use stored information
        if (moveType == MoveType::EXACT) {
            return score;
        } else if (moveType == MoveType::LOWERBOUND) {
            alpha = std::max(alpha, score);
        } else if (moveType == MoveType::UPPERBOUND) {
            beta = std::min(beta, score);
        }
        
        if (alpha >= beta) {
            pruningCount++;
            return score;
        }
    }
    
    Occupant currentPlayer = maximizingPlayer ? Occupant::BLACK : Occupant::WHITE;
    std::vector<Move> possibleMoves = board.generateMoves(currentPlayer);
    
    // Game over check: no legal moves
    if (possibleMoves.empty())
        return maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    // Get best move from TT (for move ordering)
    Move ttBestMove;
    bool hasTTMove = transpositionTable.getBestMove(board, ttBestMove);

    // Order moves using the new ordering function
    orderMoves(possibleMoves, board, currentPlayer, hasTTMove ? ttBestMove : Move());
        
    MoveType entryType = MoveType::UPPERBOUND;
    Move localBestMove;
    
    if (maximizingPlayer) {
        int value = std::numeric_limits<int>::min();
        for (const Move& move : possibleMoves) {
            Board tempBoard = board;
            tempBoard.applyMove(move);
            int eval = minimax(tempBoard, depth - 1, alpha, beta, false);
            
            if (eval > value) {
                value = eval;
                localBestMove = move;  // Remember best move for TT
            }
            
            alpha = std::max(alpha, value);
            if (beta <= alpha)
                pruningCount++;
                break;  // Beta cutoff
        }

        // Update TT entry type
        if (value <= origAlpha) {
            entryType = MoveType::UPPERBOUND;
        } else if (value >= beta) {
            entryType = MoveType::LOWERBOUND;
        } else {
            entryType = MoveType::EXACT;
        }
        
        // Store in transposition table
        transpositionTable.storeEntry(board, depth, value, entryType, localBestMove);

        return value;
    } else {
        int value = std::numeric_limits<int>::max();
        for (const Move& move : possibleMoves) {
            Board tempBoard = board;
            tempBoard.applyMove(move);
            int eval = minimax(tempBoard, depth - 1, alpha, beta, true);
            
            if (eval < value) {
                value = eval;
                localBestMove = move;  // Remember best move for TT
            }
            
            beta = std::min(beta, value);
            if (beta <= alpha)
                break;  // Alpha cutoff
        }

        // Update TT entry type
        if (value <= origAlpha) {
            entryType = MoveType::UPPERBOUND;
        } else if (value >= beta) {
            entryType = MoveType::LOWERBOUND;
        } else {
            entryType = MoveType::EXACT;
        }
        
        // Store in transposition table
        transpositionTable.storeEntry(board, depth, value, entryType, localBestMove);

        return value;
    }
}

AbaloneAI::AbaloneAI(int depth, int timeLimitMs, size_t ttSizeInMB)
    : maxDepth(depth), nodesEvaluated(0), timeLimit(timeLimitMs), 
      timeoutOccurred(false), transpositionTable(ttSizeInMB) {}

std::pair<Move, int> AbaloneAI::findBestMove(Board& board) {
    nodesEvaluated = 0;
    timeoutOccurred = false;
    startTime = std::chrono::high_resolution_clock::now();

    // Clear transposition table before a new search
    // transpositionTable.clearTable();
    transpositionTable.incrementAge();
    
    Occupant currentPlayer = board.nextToMove;
    bool maximizingPlayer = (currentPlayer == Occupant::BLACK);
    std::vector<Move> possibleMoves = board.generateMoves(currentPlayer);
    
    if (possibleMoves.empty()) {
        Move noMove;
        return std::make_pair(noMove, 0);
    }
    
    // Order the root moves
    Move ttBestMove;
    bool hasTTMove = transpositionTable.getBestMove(board, ttBestMove);
    orderMoves(possibleMoves, board, currentPlayer, hasTTMove ? ttBestMove : Move());
    
    Move bestMove = possibleMoves[0];
    int bestScore = maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    
    // Gather statistics for move ordering effectiveness
    int totalMoves = 0;
    int bestMoveIndex = 0;
    
    for (size_t i = 0; i < possibleMoves.size(); i++) {
        const Move& move = possibleMoves[i];
        totalMoves++;
        
        Board tempBoard = board;
        tempBoard.applyMove(move);
        int score = minimax(tempBoard, maxDepth - 1,
                            std::numeric_limits<int>::min(),
                            std::numeric_limits<int>::max(),
                            !maximizingPlayer);
        if ((maximizingPlayer && score > bestScore) || (!maximizingPlayer && score < bestScore)) {
            bestScore = score;
            bestMove = move;
            bestMoveIndex = i;
        }
        if (isTimeUp()) {
            timeoutOccurred = true;
            break;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - startTime).count();
    std::cout << "Nodes evaluated: " << nodesEvaluated << std::endl;
    std::cout << "Time taken: " << elapsed << " ms" << std::endl;
    std::cout << "Timeout occurred: " << (timeoutOccurred ? "Yes" : "No") << std::endl;
    std::cout << "Best move score: " << bestScore << std::endl;
    
    // Report move ordering effectiveness
    if (totalMoves > 0) {
        std::cout << "Move ordering effectiveness: best move was #" << (bestMoveIndex + 1) 
                  << " out of " << totalMoves << " moves" << std::endl;
        double effectiveness = 100.0 * (1.0 - static_cast<double>(bestMoveIndex) / totalMoves);
        std::cout << "Move ordering efficiency: " << effectiveness << "%" << std::endl;
    }
    
    return std::make_pair(bestMove, bestScore);
}

std::pair<Move, int> AbaloneAI::findBestMoveIterativeDeepening(Board& board, int maxSearchDepth) {
    nodesEvaluated = 0;
    timeoutOccurred = false;
    startTime = std::chrono::high_resolution_clock::now();
    
    Move bestMove;
    int bestScore = 0;
    bool foundMove = false;
    
    for (int depth = 1; depth <= maxSearchDepth; depth++) {
        std::cout << "Searching at depth " << depth << "..." << std::endl;
        timeoutOccurred = false;
        int originalMaxDepth = maxDepth;
        maxDepth = depth;
        
        auto result = findBestMove(board);
        
        maxDepth = originalMaxDepth;
        if (!timeoutOccurred) {
            bestMove = result.first;
            bestScore = result.second;
            foundMove = true;
            std::cout << "Completed depth " << depth << std::endl;
        } else {
            std::cout << "Timeout at depth " << depth << ", using previous result" << std::endl;
            break;
        }
        if (isTimeUp())
            break;
    }
    
    if (!foundMove) {
        std::cout << "Warning: No complete depth search finished. Using 1-ply search." << std::endl;
        maxDepth = 1;
        auto result = findBestMove(board);
        bestMove = result.first;
        bestScore = result.second;
    }

    // At the end of search, print TT usage statistics
    std::cout << "Transposition table usage: " << transpositionTable.getUsage() << "%" << std::endl;
    std::cout << "TT hit rate: " << transpositionTable.getHitRate() << "%" << std::endl;

    std::cout << "Pruning count: " << pruningCount << std::endl;
    
    return std::make_pair(bestMove, bestScore);
}