#include "AbaloneAI.h"
#include "Board.h"
#include <limits>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>
#include <future>
#include <stack>

// Evaluate the board position from BLACK's perspective
// Add this constant
const int STARTING_MARBLES = 14; // Standard Abalone has 14 marbles per side
const int PUSHED_MARBLES = 6;

// Get dynamic weights based on game progress
void AbaloneAI::getDynamicWeights(float gameProgress, int& marbleValue, int& centerValue, int& cohesionValue, int& edgeValue, int& threatValue, int& mobilityValue) {
    marbleValue = MARBLE_VALUE;
    centerValue = 20;
    cohesionValue = 5;
    edgeValue = 10;
    threatValue = 5;
    mobilityValue = 0;

    // Early game
    if (gameProgress < 0.333f) {
        marbleValue += 10;
        centerValue += 20;
        cohesionValue += 10;
        edgeValue += 5;

    // Midgame
    } else if (gameProgress <= 0.666f) {
        marbleValue += 5;
        centerValue += 10;
        cohesionValue += 5;
        edgeValue += 10;
        threatValue += 5;
        mobilityValue += 5;

    // Late game
    } else {
        marbleValue += 10;
        centerValue -= 5;
        cohesionValue += 15;
        edgeValue += 15;
        threatValue += 10;
    }
}

// Modify evaluatePosition to adjust weights based on game phase
int AbaloneAI::evaluatePosition(const Board& board) {
    nodesEvaluated++;

    // Count marbles for each side
    int blackMarbles = 0;
    int whiteMarbles = 0;
    for (int i = 0; i < Board::NUM_CELLS; i++) {
        if (board.occupant[i] == Occupant::EMPTY)
            continue;

        if (board.occupant[i] == Occupant::BLACK)
            blackMarbles++;
        else if (board.occupant[i] == Occupant::WHITE)
            whiteMarbles++;
    }

    // Determine game phase based on marble count
    int minMarbles = std::min(blackMarbles, whiteMarbles);
    float gameProgress = static_cast<float>(STARTING_MARBLES - minMarbles) / PUSHED_MARBLES;
    gameProgress = std::max(0.0f, std::min(gameProgress, 1.0f));  // Clamp to [0,1]

    // Dynamically adjust evaluation weights based on game progress
    int marbleValue, centerValue, cohesionValue, edgeValue, threatValue, mobilityValue;
    getDynamicWeights(gameProgress, marbleValue, centerValue, cohesionValue, edgeValue, threatValue, mobilityValue);

    // Calculate base score from marble count
    int score = (blackMarbles - whiteMarbles) * marbleValue;

    // Center control (same as previous implementation)
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

    score += (blackCenterControl - whiteCenterControl) * centerValue;

    // Group cohesion
    int blackCohesion = calculateCohesion(board, Occupant::BLACK);
    int whiteCohesion = calculateCohesion(board, Occupant::WHITE);
    score += (blackCohesion - whiteCohesion) * cohesionValue;

    // Edge danger
    int blackEdgeVulnerability = calculateEdgeDanger(board, Occupant::BLACK);
    int whiteEdgeVulnerability = calculateEdgeDanger(board, Occupant::WHITE);
    score -= (blackEdgeVulnerability - whiteEdgeVulnerability) * edgeValue;

    // Threat potential
    int blackThreat = calculateThreatPotential(board, Occupant::BLACK);
    int whiteThreat = calculateThreatPotential(board, Occupant::WHITE);
    score += (blackThreat - whiteThreat) * threatValue;

    // Mobility
    int blackMobility = calculateMobility(board, Occupant::BLACK);
    int whiteMobility = calculateMobility(board, Occupant::WHITE);
    score += (blackMobility - whiteMobility) * mobilityValue;

    return score;
}

// Evaluate a move quickly for ordering purposes
int AbaloneAI::evaluateMove(const Board& board, const Move& move, Occupant side) {
    int score = 0;

    // Prioritize captures
    if (move.pushCount > 0) {
        score += 1000 * move.pushCount;  // Higher score for more captures
    }

    // Dynamic weights based on game progress
    int marbleValue, centerValue, cohesionValue, edgeValue, threatValue, mobilityValue;
    getDynamicWeights(0.5f, marbleValue, centerValue, cohesionValue, edgeValue, threatValue, mobilityValue);

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
        score += (beforeCentralization - afterCentralization) * centerValue;
    }

    // Prioritize group-forming moves
    int beforeCohesion = calculateCohesion(board, side);
    int afterCohesion = calculateCohesion(tempBoard, side);
    score += (afterCohesion - beforeCohesion) * cohesionValue;

    // Penalize moves that put marbles in danger
    int beforeDanger = calculateEdgeDanger(board, side);
    int afterDanger = calculateEdgeDanger(tempBoard, side);
    score -= (afterDanger - beforeDanger) * edgeValue;

    // Calculate threat potential
    int beforeThreat = calculateThreatPotential(board, side);
    int afterThreat = calculateThreatPotential(tempBoard, side);
    score += (afterThreat - beforeThreat) * threatValue;

    // Calculate mobility
    int beforeMobility = calculateMobility(board, side);
    int afterMobility = calculateMobility(tempBoard, side);
    score += (afterMobility - beforeMobility) * mobilityValue;

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

int AbaloneAI::calculateMobility(const Board& board, Occupant side) {
    int mobilityScore = 0;

    for (int i = 0; i < Board::NUM_CELLS; i++) {
        if (board.occupant[i] == side) {
            for (int d = 0; d < Board::NUM_DIRECTIONS; d++) {
                int neighbor = board.neighbors[i][d];
                if (neighbor >= 0 && board.occupant[neighbor] == Occupant::EMPTY) {
                    mobilityScore++;
                }
            }
        }
    }

    return mobilityScore;
}

// Calculate the cohesion of a group of marbles
int AbaloneAI::calculateCohesion(const Board& board, Occupant side) {
    std::vector<bool> visited(Board::NUM_CELLS, false);
    int maxSize = 0;
    for (int i = 0; i < Board::NUM_CELLS; i++) {
        if (board.occupant[i] == side && !visited[i]) {
            int size = 0;
            std::stack<int> s;
            s.push(i);
            visited[i] = true;
            while (!s.empty()) {
                int cell = s.top();
                s.pop();
                size++;
                for (int d = 0; d < Board::NUM_DIRECTIONS; d++) {
                    int n = board.neighbors[cell][d];
                    if (n >= 0 && board.occupant[n] == side && !visited[n]) {
                        s.push(n);
                        visited[n] = true;
                    }
                }
            }
            maxSize = std::max(maxSize, size);
        }
    }
    return maxSize;
}

// Calculate the number of marbles on the edge of the board
int AbaloneAI::calculateEdgeDanger(const Board& board, Occupant side) {
    int vulnerability = 0;
    for (int i = 0; i < Board::NUM_CELLS; i++) {
        if (board.occupant[i] == side) {
            bool onEdge = false;
            for (int d = 0; d < Board::NUM_DIRECTIONS; d++) {
                if (board.neighbors[i][d] < 0) {
                    onEdge = true;
                    break;
                }
            }
            if (onEdge) {
                int support = 0;
                for (int d = 0; d < Board::NUM_DIRECTIONS; d++) {
                    int n = board.neighbors[i][d];
                    if (n >= 0 && board.occupant[n] == side) support++;
                }
                vulnerability += std::max(0, 3 - support);
            }
        }
    }
    return vulnerability;
}

// Calculate the threat potential of a player
int AbaloneAI::calculateThreatPotential(const Board& board, Occupant side) {
    Occupant opponent = (side == Occupant::BLACK) ? Occupant::WHITE : Occupant::BLACK;
    int vulnerability = 0;
    for (int i = 0; i < Board::NUM_CELLS; i++) {
        if (board.occupant[i] == opponent && board.isOnEdge(i)) {
            int support = 0;
            for (int d = 0; d < Board::NUM_DIRECTIONS; d++) {
                int n = board.neighbors[i][d];
                if (n >= 0 && board.occupant[n] == opponent) support++;
            }
            vulnerability += std::max(0, 3 - support);  // Penalize low support
        }
    }
    return vulnerability;
}

bool AbaloneAI::isTimeUp() {
    if (timeLimit <= 0)
        return false;
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
    return elapsed >= timeLimit;
}

// Helper method to update killer moves
void AbaloneAI::updateKillerMove(const Move& move, int depth) {
    // Don't store captures as killer moves (they're already prioritized)
    if (move.pushCount > 0)
        return;

    // Don't store the move if it's already the first killer move
    if (killerMoves[depth][0] == move)
        return;

    // Shift the existing killer move to the second position
    killerMoves[depth][1] = killerMoves[depth][0];

    // Store the new killer move in the first position
    killerMoves[depth][0] = move;
}

// Helper function to check if a move is a killer move
bool AbaloneAI::isKillerMove(const Move& move, int depth) const {
    return (depth < killerMoves.size() &&
        (killerMoves[depth][0] == move || killerMoves[depth][1] == move));
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

    // Score each move
    for (const Move& move : moves) {
        int moveScore = 0;

        // 1. Highest priority: Transposition table move
        if (move == ttMove) {
            moveScore = 100000;  // Very high score
        }
        // 2. Second priority: Killer moves
        else if (isKillerMove(move, maxDepth)) {
            moveScore = 10000;  // High score, but lower than TT move

            // First killer move gets higher priority than second
            if (move == killerMoves[maxDepth][0]) {
                moveScore += 1000;
            }
        }
        // 3. Third priority: Move evaluation heuristic
        else {
            moveScore = evaluateMove(board, move, side);
        }

        for (int idx : move.marbleIndices) {
            moveScore += historyTable[idx][move.direction];
        }

        scoredMoves.push_back(ScoredMove(move, moveScore));
    }

    // Sort moves by score
    std::sort(scoredMoves.begin(), scoredMoves.end());

    // Update the original vector with sorted moves
    for (size_t i = 0; i < moves.size(); i++) {
        moves[i] = scoredMoves[i].move;
    }
}

int AbaloneAI::minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer) {
    Occupant currentPlayer = maximizingPlayer ? Occupant::BLACK : Occupant::WHITE;

    if (isTimeUp()) {
        timeoutOccurred = true;
        return evaluatePosition(board);
    }
    if (depth == 0) {
        std::vector<Move> captureMoves = board.generateCaptureMoves(currentPlayer);
        if (!captureMoves.empty()) {
            return quiescenceSearch(board, alpha, beta, currentPlayer);
        }
        return evaluatePosition(board);
    }

    // Transposition Table Check
    int origAlpha = alpha;
    int origBeta = beta;
    Move bestMove;
    int score;
    MoveType moveType;

    if (transpositionTable.probeEntry(board, depth, score, moveType, bestMove)) {
        if (moveType == MoveType::EXACT) {
            return score;
        }
        else if (moveType == MoveType::LOWERBOUND) {
            alpha = std::max(alpha, score);
        }
        else if (moveType == MoveType::UPPERBOUND) {
            beta = std::min(beta, score);
        }
        if (alpha >= beta) {
            pruningCount++;
            return score;
        }
    }

    // Generate possible moves
    std::vector<Move> possibleMoves = board.generateMoves(currentPlayer);

    if (possibleMoves.empty()) {
        return maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    }

    // Move ordering
    Move ttBestMove;
    bool hasTTMove = transpositionTable.getBestMove(board, ttBestMove);
    orderMoves(possibleMoves, board, currentPlayer, hasTTMove ? ttBestMove : Move());

    MoveType entryType = MoveType::UPPERBOUND;
    Move localBestMove;
    int value = maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    // PVS: Principal Variation Search
    bool firstMove = true;
    for (const Move& move : possibleMoves) {
        Board tempBoard = board;
        tempBoard.applyMove(move);

        int eval;
        if (firstMove) {
            // Full window search for the first move
            eval = minimax(tempBoard, depth - 1, alpha, beta, !maximizingPlayer);
            firstMove = false;
        }
        else {
            // Null window search (PVS)
            eval = minimax(tempBoard, depth - 1, alpha, alpha + 1, !maximizingPlayer);
            if (eval > alpha && eval < beta) {
                // Full re-search if null-window fails
                eval = minimax(tempBoard, depth - 1, alpha, beta, !maximizingPlayer);
            }
        }

        if (maximizingPlayer) {
            if (eval > value) {
                value = eval;
                localBestMove = move;
            }
            alpha = std::max(alpha, eval);
        }
        else {
            if (eval < value) {
                value = eval;
                localBestMove = move;
            }
            beta = std::min(beta, eval);
        }

        if (beta <= alpha) {
            pruningCount++;
            updateKillerMove(move, depth);
            for (int idx : move.marbleIndices) {
                historyTable[idx][move.direction] += depth * depth;
            }
            break;
        }
    }

    // Store in transposition table
    if (value <= origAlpha) {
        entryType = MoveType::UPPERBOUND;
    }
    else if (value >= origBeta) {
        entryType = MoveType::LOWERBOUND;
    }
    else {
        entryType = MoveType::EXACT;
    }
    transpositionTable.storeEntry(board, depth, value, entryType, localBestMove);

    return value;
}

int AbaloneAI::quiescenceSearch(Board& board, int alpha, int beta, Occupant side) {
    int standPat = evaluatePosition(board);
    if (standPat >= beta) return beta;
    if (alpha < standPat) alpha = standPat;

    std::vector<Move> captureMoves = board.generateCaptureMoves(side);  // Assume this exists or adapt
    for (const Move& move : captureMoves) {
        Board tempBoard = board;
        tempBoard.applyMove(move);
        int score = -quiescenceSearch(tempBoard, -beta, -alpha, (side == Occupant::BLACK) ? Occupant::WHITE : Occupant::BLACK);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

AbaloneAI::AbaloneAI(int depth, int timeLimitMs, size_t ttSizeInMB)
    : maxDepth(depth), nodesEvaluated(0), timeLimit(timeLimitMs),
    timeoutOccurred(false), transpositionTable(ttSizeInMB),
    killerMoves(depth + 1) {
    pruningCount = 0;
}

std::pair<Move, int> AbaloneAI::findBestMove(Board& board) {
    nodesEvaluated = 0;
    timeoutOccurred = false;
    startTime = std::chrono::high_resolution_clock::now();

    transpositionTable.incrementAge();
    killerMoves = std::vector<std::array<Move, MAX_KILLER_MOVES>>(maxDepth + 1);
    historyTable = std::vector<std::vector<int>>(Board::NUM_CELLS, std::vector<int>(Board::NUM_DIRECTIONS, 0));

    Occupant currentPlayer = board.nextToMove;
    bool maximizingPlayer = (currentPlayer == Occupant::BLACK);
    std::vector<Move> possibleMoves = board.generateMoves(currentPlayer);

    if (possibleMoves.empty()) {
        return std::make_pair(Move(), 0);
    }

    Move ttBestMove;
    bool hasTTMove = transpositionTable.getBestMove(board, ttBestMove);
    orderMoves(possibleMoves, board, currentPlayer, hasTTMove ? ttBestMove : Move());

    Move bestMove = possibleMoves[0];
    int bestScore = maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

// Sets thread count to 8 
// ======================
// WARNING!!! THIS IS SYSTEM SPECIFIC
// ======================
    int threadCount = std::min(8, (int)possibleMoves.size());
    std::vector<std::future<std::pair<int, Move>>> futures;
    
    for (int i = 0; i < threadCount; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            const Move& move = possibleMoves[i];
            Board tempBoard = board;
            tempBoard.applyMove(move);
            int score = this->minimax(tempBoard, maxDepth - 1,
                                      std::numeric_limits<int>::min(),
                                      std::numeric_limits<int>::max(),
                                      !maximizingPlayer);
            return std::make_pair(score, move);
        }));
    }

//  // Changes thread count
//     std::vector<std::future<std::pair<int, Move>>> futures;
//     int threadCount = std::min((int)possibleMoves.size(), static_cast<int>(std::thread::hardware_concurrency()));
    
//     for (const Move& move : possibleMoves) {
//         futures.push_back(std::async(std::launch::async, [&board, move, this, maximizingPlayer]() {
//             Board tempBoard = board;
//             tempBoard.applyMove(move);
//             int score = this->minimax(tempBoard, maxDepth - 1,
//                                       std::numeric_limits<int>::min(),
//                                       std::numeric_limits<int>::max(),
//                                       !maximizingPlayer);
//             return std::make_pair(score, move);
//         }));
//     }

    for (auto& f : futures) {
        if (isTimeUp()) {
            timeoutOccurred = true;
            break;
        }
        auto [score, move] = f.get();
        if ((maximizingPlayer && score > bestScore) || (!maximizingPlayer && score < bestScore)) {
            bestScore = score;
            bestMove = move;
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

        // Check if total elapsed time exceeds the time limit
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

        if (elapsed >= timeLimit) {
            std::cout << "Total time limit exceeded. Stopping search." << std::endl;
            break;
        }

        // Adjust remaining time for this depth
        int remainingTime = timeLimit - elapsed;

        // Temporarily modify the time limit for this depth's search
        int originalTimeLimit = timeLimit;
        timeLimit = remainingTime;

        int originalMaxDepth = maxDepth;
        maxDepth = depth;

        auto result = findBestMove(board);

        // Restore original time limit and max depth
        timeLimit = originalTimeLimit;
        maxDepth = originalMaxDepth;

        if (!timeoutOccurred) {
            bestMove = result.first;
            bestScore = result.second;
            foundMove = true;
            std::cout << "Completed depth " << depth << std::endl;
        }
        else {
            std::cout << "Timeout at depth " << depth << ", using previous result" << std::endl;
            break;
        }
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