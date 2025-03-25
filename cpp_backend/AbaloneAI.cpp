#include "AbaloneAI.h"
#include "Board.h"
#include <limits>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <valarray>

int AbaloneAI::PRUNES_OCCURED = 0;


long long AbaloneAI::packCoord(int m, int y) {
    return (static_cast<long long>(m) << 32) ^ (static_cast<long long>(y) & 0xffffffff);
}



int AbaloneAI::evaluateCounterDefensively(const Board& board) {
    double black_points = 0;
    double white_points = 0;

    const int reversed_points[] = {5, 4, 3, 2, 1};

    for (const auto& black_marble : board.blackOccupantsCoords) {
        black_points += 10;
        int squared_sum = std::pow( black_marble.first, 2) + std::pow( black_marble.second, 2);
        int square_root = static_cast<int>(std::sqrt(squared_sum));
        if (square_root >= 0 && square_root < 5)
            black_points += reversed_points[square_root];

        long long key = packCoord(black_marble.first, black_marble.second);
        int marble_index = Board::s_coordToIndex.at(key);  // safer with `.at()` or check existence

        for (int i = 0; i < Board::NUM_DIRECTIONS; ++i) {
            int neighborIndex = board.neighbors[marble_index][i];
            if (neighborIndex >= 0) {
                if (board.occupant[neighborIndex] == Occupant::BLACK) {
                    black_points += 1;
                }
            } else {
                black_points -= 1;
                white_points += 1;
            }
        }
    }

    for (const auto& white_marble : board.whiteOccupantsCoords) {
        white_points += 10;
        int squared_sum = std::pow( white_marble.first, 2) + std::pow( white_marble.second, 2);
        int square_root = static_cast<int>(std::sqrt(squared_sum));
        if (square_root >= 0 && square_root < 5)
            white_points += reversed_points[square_root];

        long long key = packCoord(white_marble.first, white_marble.second);
        int marble_index = Board::s_coordToIndex.at(key);

        for (int i = 0; i < Board::NUM_DIRECTIONS; ++i) {
            int neighborIndex = board.neighbors[marble_index][i];
            if (neighborIndex >= 0) {
                if (board.occupant[neighborIndex] == Occupant::WHITE) {
                    white_points += 1;
                }
            } else {
                white_points -= 1;
                black_points += 1;
            }
        }
    }

    return static_cast<int>(black_points - white_points);
}


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

bool AbaloneAI::isTimeUp() {
    if (timeLimit <= 0)
        return false;
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
    return elapsed >= timeLimit;
}

int AbaloneAI::minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer) {
    if (isTimeUp()) {
        timeoutOccurred = true;
        return evaluateCounterDefensively(board);
    }

    if (depth == 0)
        return evaluateCounterDefensively(board);

    // Transposition table lookup
    int origAlpha = alpha;
    Move bestMove;
    int ttScore;
    MoveType moveType;

    if (transpositionTable.probeEntry(board, depth, ttScore, moveType, bestMove)) {
        if (moveType == MoveType::EXACT)
            return ttScore;
        else if (moveType == MoveType::LOWERBOUND)
            alpha = std::max(alpha, ttScore);
        else if (moveType == MoveType::UPPERBOUND)
            beta = std::min(beta, ttScore);

        if (alpha >= beta)
            return ttScore;
    }

    Occupant currentPlayer = maximizingPlayer ? Occupant::BLACK : Occupant::WHITE;
    std::vector<Move> possibleMoves = board.generateMoves(currentPlayer);

    if (possibleMoves.empty())
        return maximizingPlayer ? std::numeric_limits<int>::min()
                                : std::numeric_limits<int>::max();

    // Score and cache board states for each move
    std::vector<std::tuple<Move, int, Board>> scoredMoves;
    for (const Move& move : possibleMoves) {
        Board tempBoard = board;
        tempBoard.applyMove(move);
        int moveScore = evaluateCounterDefensively(tempBoard);
        scoredMoves.emplace_back(move, moveScore, tempBoard);
    }

    // Sort by heuristic score
    std::sort(scoredMoves.begin(), scoredMoves.end(), [&](const auto& a, const auto& b) {
        return maximizingPlayer ? std::get<1>(a) > std::get<1>(b)
                                : std::get<1>(a) < std::get<1>(b);
    });

    // Rebuild move and board vectors
    std::vector<Move> orderedMoves;
    std::vector<Board> correspondingBoards;
    for (const auto& tup : scoredMoves) {
        orderedMoves.push_back(std::get<0>(tup));
        correspondingBoards.push_back(std::get<2>(tup));
    }

    // Move ordering using TT
    Move ttBestMove;
    bool hasTTMove = transpositionTable.getBestMove(board, ttBestMove);
    if (hasTTMove) {
        auto it = std::find(orderedMoves.begin(), orderedMoves.end(), ttBestMove);
        if (it != orderedMoves.end()) {
            size_t idx = std::distance(orderedMoves.begin(), it);
            std::rotate(orderedMoves.begin(), it, it + 1);
            std::rotate(correspondingBoards.begin(), correspondingBoards.begin() + idx,
                        correspondingBoards.begin() + idx + 1);
        }
    }

    MoveType entryType = MoveType::UPPERBOUND;
    Move localBestMove;
    int bestEval = maximizingPlayer ? std::numeric_limits<int>::min()
                                    : std::numeric_limits<int>::max();

    for (size_t i = 0; i < orderedMoves.size(); ++i) {
        const Move& move = orderedMoves[i];
        Board &childBoard = correspondingBoards[i];

        int eval = minimax(childBoard, depth - 1, alpha, beta, !maximizingPlayer);

        if (maximizingPlayer) {
            if (eval > bestEval) {
                bestEval = eval;
                localBestMove = move;
            }
            alpha = std::max(alpha, bestEval);
        } else {
            if (eval < bestEval) {
                bestEval = eval;
                localBestMove = move;
            }
            beta = std::min(beta, bestEval);
        }

        if (beta <= alpha) {
            PRUNES_OCCURED++;
            break; // Prune
        }
    }

    // Determine TT entry type
    if (bestEval <= origAlpha)
        entryType = MoveType::UPPERBOUND;
    else if (bestEval >= beta)
        entryType = MoveType::LOWERBOUND;
    else
        entryType = MoveType::EXACT;

    transpositionTable.storeEntry(board, depth, bestEval, entryType, localBestMove);
    return bestEval;
}


AbaloneAI::AbaloneAI(int depth, int timeLimitMs, size_t ttSizeInMB)
    : maxDepth(depth), nodesEvaluated(0), timeLimit(timeLimitMs), 
      timeoutOccurred(false), transpositionTable(ttSizeInMB) {}

std::pair<Move, int> AbaloneAI::findBestMove(Board& board) {
    nodesEvaluated = 0;
    timeoutOccurred = false;
    startTime = std::chrono::high_resolution_clock::now();

    // Clear transposition table before a new search`
    transpositionTable.clearTable();
    
    Occupant currentPlayer = board.nextToMove;
    bool maximizingPlayer = (currentPlayer == Occupant::BLACK);
    std::vector<Move> possibleMoves = board.generateMoves(currentPlayer);
    
    if (possibleMoves.empty()) {
        Move noMove;
        return std::make_pair(noMove, 0);
    }
    
    Move bestMove = possibleMoves[0];
    int bestScore = maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    
    for (const Move& move : possibleMoves) {
        Board tempBoard = board;
        tempBoard.applyMove(move);
        int score = minimax(tempBoard, maxDepth - 1,
                            std::numeric_limits<int>::min(),
                            std::numeric_limits<int>::max(),
                            !maximizingPlayer);
        if ((maximizingPlayer && score > bestScore) || (!maximizingPlayer && score < bestScore)) {
            bestScore = score;
            bestMove = move;
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
            std::cout << "Prunes Occured: " << PRUNES_OCCURED << std::endl;
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
    
    return std::make_pair(bestMove, bestScore);
}







//old version
#include "AbaloneAI.h"
#include "Board.h"
#include <limits>
#include <algorithm>
#include <chrono>
#include <iostream>

// Evaluate the board position from BLACK's perspective
// int AbaloneAI::evaluatePosition(const Board& board) {
//     nodesEvaluated++;
//
//     // Count marbles for each side
//     int blackMarbles = 0;
//     int whiteMarbles = 0;
//     for (int i = 0; i < Board::NUM_CELLS; i++) {
//         if (board.occupant[i] == Occupant::BLACK)
//             blackMarbles++;
//         else if (board.occupant[i] == Occupant::WHITE)
//             whiteMarbles++;
//     }
//
//     // Basic score: difference in marble counts
//     int score = (blackMarbles - whiteMarbles) * MARBLE_VALUE;
//
//     // Component 1: Center control bonus
//     int blackCenterControl = 0;
//     int whiteCenterControl = 0;
//     std::vector<int> centerCells = {
//         Board::notationToIndex("E5"),
//         Board::notationToIndex("D5"),
//         Board::notationToIndex("F5"),
//         Board::notationToIndex("E4"),
//         Board::notationToIndex("E6")
//     };
//     for (int idx : centerCells) {
//         if (idx >= 0) {
//             if (board.occupant[idx] == Occupant::BLACK)
//                 blackCenterControl++;
//             else if (board.occupant[idx] == Occupant::WHITE)
//                 whiteCenterControl++;
//         }
//     }
//     score += (blackCenterControl - whiteCenterControl) * 10;
//
//     // Component 2: Group cohesion bonus
//     int blackCohesion = calculateCohesion(board, Occupant::BLACK);
//     int whiteCohesion = calculateCohesion(board, Occupant::WHITE);
//     score += (blackCohesion - whiteCohesion) * 5;
//
//     // Component 3: Edge danger penalty
//     int blackEdgeDanger = calculateEdgeDanger(board, Occupant::BLACK);
//     int whiteEdgeDanger = calculateEdgeDanger(board, Occupant::WHITE);
//     score -= (blackEdgeDanger - whiteEdgeDanger) * 15;
//
//     return score;
// }
//
// int AbaloneAI::calculateCohesion(const Board& board, Occupant side) {
//     int cohesion = 0;
//     for (int i = 0; i < Board::NUM_CELLS; i++) {
//         if (board.occupant[i] == side) {
//             for (int d = 0; d < Board::NUM_DIRECTIONS; d++) {
//                 int neighbor = board.neighbors[i][d];
//                 if (neighbor >= 0 && board.occupant[neighbor] == side)
//                     cohesion++;
//             }
//         }
//     }
//     return cohesion;
// }
//
// int AbaloneAI::calculateEdgeDanger(const Board& board, Occupant side) {
//     int edgeCount = 0;
//     for (int i = 0; i < Board::NUM_CELLS; i++) {
//         if (board.occupant[i] == side) {
//             bool onEdge = false;
//             for (int d = 0; d < Board::NUM_DIRECTIONS; d++) {
//                 if (board.neighbors[i][d] < 0) {  // neighbor off-board
//                     onEdge = true;
//                     break;
//                 }
//             }
//             if (onEdge)
//                 edgeCount++;
//         }
//     }
//     return edgeCount;
// }
//
// bool AbaloneAI::isTimeUp() {
//     if (timeLimit <= 0)
//         return false;
//     auto now = std::chrono::high_resolution_clock::now();
//     auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
//     return elapsed >= timeLimit;
// }
//
// int AbaloneAI::minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer) {
//     if (isTimeUp()) {
//         timeoutOccurred = true;
//         return evaluatePosition(board);
//     }
//     if (depth == 0)
//         return evaluatePosition(board);
//
//     // Check transposition table first
//     int origAlpha = alpha;
//     int origBeta = beta;
//     Move bestMove;
//     int score;
//     MoveType moveType;
//
//     if (transpositionTable.probeEntry(board, depth, score, moveType, bestMove)) {
//         // TT hit - use stored information
//         if (moveType == MoveType::EXACT) {
//             return score;
//         } else if (moveType == MoveType::LOWERBOUND) {
//             alpha = std::max(alpha, score);
//         } else if (moveType == MoveType::UPPERBOUND) {
//             beta = std::min(beta, score);
//         }
//
//         if (alpha >= beta) {
//             return score;
//         }
//     }
//
//     Occupant currentPlayer = maximizingPlayer ? Occupant::BLACK : Occupant::WHITE;
//     std::vector<Move> possibleMoves = board.generateMoves(currentPlayer);
//
//     // Game over check: no legal moves
//     if (possibleMoves.empty())
//         return maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
//
//
//     // Get best move from TT (for move ordering)
//     Move ttBestMove;
//     bool hasTTMove = transpositionTable.getBestMove(board, ttBestMove);
//
//     // Move ordering: try TT move first if available
//     if (hasTTMove) {
//         // Move ttBestMove to the front of possibleMoves
//         auto it = std::find_if(possibleMoves.begin(), possibleMoves.end(),
//                                 [&ttBestMove](const Move& m) {
//                                     return m == ttBestMove;
//                                 });
//         if (it != possibleMoves.end()) {
//             std::rotate(possibleMoves.begin(), it, it + 1);
//         }
//     }
//
//     MoveType entryType = MoveType::UPPERBOUND;
//     Move localBestMove;
//
//     if (maximizingPlayer) {
//         int value = std::numeric_limits<int>::min();
//         for (const Move& move : possibleMoves) {
//             Board tempBoard = board;
//             tempBoard.applyMove(move);
//             int eval = minimax(tempBoard, depth - 1, alpha, beta, false);
//             value = std::max(value, eval);
//             alpha = std::max(alpha, value);
//             if (beta <= alpha)
//                 break;  // Beta cutoff
//         }
//
//         // Update TT entry type
//         if (value <= origAlpha) {
//             entryType = MoveType::UPPERBOUND;
//         } else if (value >= beta) {
//             entryType = MoveType::LOWERBOUND;
//         } else {
//             entryType = MoveType::EXACT;
//         }
//
//         // Store in transposition table
//         transpositionTable.storeEntry(board, depth, value, entryType, localBestMove);
//
//         return value;
//     } else {
//         int value = std::numeric_limits<int>::max();
//         for (const Move& move : possibleMoves) {
//             Board tempBoard = board;
//             tempBoard.applyMove(move);
//             int eval = minimax(tempBoard, depth - 1, alpha, beta, true);
//             value = std::min(value, eval);
//             beta = std::min(beta, value);
//             if (beta <= alpha)
//                 break;  // Alpha cutoff
//         }
//
//         // Update TT entry type
//         if (value <= origAlpha) {
//             entryType = MoveType::UPPERBOUND;
//         } else if (value >= beta) {
//             entryType = MoveType::LOWERBOUND;
//         } else {
//             entryType = MoveType::EXACT;
//         }
//
//         // Store in transposition table
//         transpositionTable.storeEntry(board, depth, value, entryType, localBestMove);
//
//         return value;
//     }
// }
//
// AbaloneAI::AbaloneAI(int depth, int timeLimitMs, size_t ttSizeInMB)
//     : maxDepth(depth), nodesEvaluated(0), timeLimit(timeLimitMs),
//       timeoutOccurred(false), transpositionTable(ttSizeInMB) {}
//
// std::pair<Move, int> AbaloneAI::findBestMove(Board& board) {
//     nodesEvaluated = 0;
//     timeoutOccurred = false;
//     startTime = std::chrono::high_resolution_clock::now();
//
//     // Clear transposition table before a new search
//     transpositionTable.clearTable();
//
//     Occupant currentPlayer = board.nextToMove;
//     bool maximizingPlayer = (currentPlayer == Occupant::BLACK);
//     std::vector<Move> possibleMoves = board.generateMoves(currentPlayer);
//
//     if (possibleMoves.empty()) {
//         Move noMove;
//         return std::make_pair(noMove, 0);
//     }
//
//     Move bestMove = possibleMoves[0];
//     int bestScore = maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
//
//     for (const Move& move : possibleMoves) {
//         Board tempBoard = board;
//         tempBoard.applyMove(move);
//         int score = minimax(tempBoard, maxDepth - 1,
//                             std::numeric_limits<int>::min(),
//                             std::numeric_limits<int>::max(),
//                             !maximizingPlayer);
//         if ((maximizingPlayer && score > bestScore) || (!maximizingPlayer && score < bestScore)) {
//             bestScore = score;
//             bestMove = move;
//         }
//         if (isTimeUp()) {
//             timeoutOccurred = true;
//             break;
//         }
//     }
//
//     auto end = std::chrono::high_resolution_clock::now();
//     auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - startTime).count();
//     std::cout << "Nodes evaluated: " << nodesEvaluated << std::endl;
//     std::cout << "Time taken: " << elapsed << " ms" << std::endl;
//     std::cout << "Timeout occurred: " << (timeoutOccurred ? "Yes" : "No") << std::endl;
//     std::cout << "Best move score: " << bestScore << std::endl;
//
//     return std::make_pair(bestMove, bestScore);
// }
//
// std::pair<Move, int> AbaloneAI::findBestMoveIterativeDeepening(Board& board, int maxSearchDepth) {
//     nodesEvaluated = 0;
//     timeoutOccurred = false;
//     startTime = std::chrono::high_resolution_clock::now();
//
//     Move bestMove;
//     int bestScore = 0;
//     bool foundMove = false;
//
//     for (int depth = 1; depth <= maxSearchDepth; depth++) {
//         std::cout << "Searching at depth " << depth << "..." << std::endl;
//         timeoutOccurred = false;
//         int originalMaxDepth = maxDepth;
//         maxDepth = depth;
//
//         auto result = findBestMove(board);
//
//         maxDepth = originalMaxDepth;
//         if (!timeoutOccurred) {
//             bestMove = result.first;
//             bestScore = result.second;
//             foundMove = true;
//             std::cout << "Completed depth " << depth << std::endl;
//         } else {
//             std::cout << "Timeout at depth " << depth << ", using previous result" << std::endl;
//             break;
//         }
//         if (isTimeUp())
//             break;
//     }
//
//     if (!foundMove) {
//         std::cout << "Warning: No complete depth search finished. Using 1-ply search." << std::endl;
//         maxDepth = 1;
//         auto result = findBestMove(board);
//         bestMove = result.first;
//         bestScore = result.second;
//     }
//
//     // At the end of search, print TT usage statistics
//     std::cout << "Transposition table usage: " << transpositionTable.getUsage() << "%" << std::endl;
//
//     return std::make_pair(bestMove, bestScore);
// }

