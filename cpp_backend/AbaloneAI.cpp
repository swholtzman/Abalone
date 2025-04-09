#include "AbaloneAI.h"
#include "Board.h"
#include <limits>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>
#include <future>
#include <random>

int STARTING_MARBLES = 14; // Number of marbles each player starts with
int WIN_THRESHOLD = 6; // Number of marbles pushed off to win
int ENDGAME = STARTING_MARBLES - WIN_THRESHOLD; // Number of marbles left for endgame

// Modify evaluatePosition to adjust weights based on game phase
int AbaloneAI::evaluatePosition(const Board& board, float gameProgress) {
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

    // Check if we're in endgame with tied scores
    bool scoresTied = blackMarbles == whiteMarbles;
    bool endgameNear = gameProgress >= 0.9f;
    bool midGame = gameProgress >= 0.3f;

    bool closeToWin = false;
    if (board.nextToMove == Occupant::BLACK) {
        closeToWin = (whiteMarbles - 1) == ENDGAME;
    }
    else {
        closeToWin = (blackMarbles - 1) == ENDGAME;
    }

    // Base marble count evaluation
    int score = (blackMarbles - whiteMarbles) * MARBLE_VALUE;

    // If we're in endgame with tied scores, dramatically increase the value of having fewer marbles
    // (which means we've pushed more off)
    if (scoresTied && endgameNear || closeToWin && endgameNear) {
        score = (blackMarbles - whiteMarbles) * MARBLE_VALUE * 10; // 10x importance
    }

    // Adjust weights based on game phase
    int marbleValue = MARBLE_VALUE;
    int centerValue = 15;
    int cohesionValue = 5;
    int edgeValue = 15;
    int threatValue = 10;

    if (gameProgress >= 0.5f) {
        // Mid-game: increase center control and cohesion values
        edgeValue = 20;
        cohesionValue = 10;
        centerValue = 20;
    }

    // Center control
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
    int blackEdgeDanger = calculateEdgeDanger(board, Occupant::BLACK);
    int whiteEdgeDanger = calculateEdgeDanger(board, Occupant::WHITE);
    score -= (blackEdgeDanger - whiteEdgeDanger) * edgeValue;

    // Threat potential
    int blackThreats = calculateThreatPotential(board, Occupant::BLACK);
    int whiteThreats = calculateThreatPotential(board, Occupant::WHITE);
    score += (blackThreats - whiteThreats) * threatValue;

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

            // Calculate where this marble ended up more accurately
            // Follow the marble through to its final position after the move
            int endIdx = idx;
            // First, find where this specific marble will end up in the move direction
            if (std::find(move.marbleIndices.begin(), move.marbleIndices.end(), idx) != move.marbleIndices.end()) {
                // Count how many marbles are ahead of this one in the move direction
                int marblesAhead = 0;
                int currentIdx = idx;
                for (int i = 0; i < move.marbleIndices.size(); i++) {
                    int nextIdx = board.neighbors[currentIdx][move.direction];
                    if (nextIdx >= 0 && std::find(move.marbleIndices.begin(), move.marbleIndices.end(), nextIdx) != move.marbleIndices.end()) {
                        marblesAhead++;
                        currentIdx = nextIdx;
                    } else {
                        break;
                    }
                }
                
                // Trace the path to the final position, accounting for any pushed marbles
                endIdx = idx;
                for (int i = 0; i <= marblesAhead; i++) {
                    int nextIdx = board.neighbors[endIdx][move.direction];
                    if (nextIdx >= 0) {
                        endIdx = nextIdx;
                    } else {
                        // If we hit the edge, the marble stays at its current position
                        // (or gets pushed off, but that's handled by the Board::applyMove)
                        break;
                    }
                }
                
                // Only evaluate distance if the marble is still on the board
                if (endIdx >= 0 && endIdx < Board::NUM_CELLS) {
                    auto afterCoord = board.s_indexToCoord[endIdx];
                    int distAfter = std::abs(afterCoord.first - centerCoord.first) +
                        std::abs(afterCoord.second - centerCoord.second);
                    afterCentralization += distAfter;
                }
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

    // Prioritize moves that increase threat potential
    int beforeThreats = calculateThreatPotential(board, side);
    int afterThreats = calculateThreatPotential(tempBoard, side);
    score += (afterThreats - beforeThreats) * 10;


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


// Calculate the cohesion of a group of marbles
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

// Calculate the number of marbles on the edge of the board
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
    bool result = elapsed >= timeLimit;

    if (result) {
        std::lock_guard<std::mutex> lock(timeoutMutex);
        timeoutOccurred = true;
    }

    return result;
}

// Helper method to update killer moves
void AbaloneAI::updateKillerMove(const Move& move, int depth) {

    std::lock_guard<std::mutex> lock(killerMovesMutex);

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
    std::lock_guard<std::mutex> lock(killerMovesMutex);
    return (depth < killerMoves.size() &&
        (killerMoves[depth][0] == move || killerMoves[depth][1] == move));
}

// Helper function to sort moves based on their evaluation
void AbaloneAI::orderMoves(std::vector<Move>& moves, const Board& board, Occupant side, const Move& ttMove, int depth) {
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
        else if (isKillerMove(move, depth)) {
            moveScore = 10000;  // High score, but lower than TT move

            // First killer move gets higher priority than second
            if (move == killerMoves[depth][0]) {
                moveScore += 1000;
            }
        }
        // 3. Third priority: Move evaluation heuristic
        else {
            moveScore = evaluateMove(board, move, side);
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

int AbaloneAI::minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, float gameProgress) {

    {
        std::lock_guard<std::mutex> lock(timeoutMutex);
        if (timeoutOccurred) {
            timeoutOccurred = true;
            return evaluatePosition(board, gameProgress);
        }
    }

    {
        std::lock_guard<std::mutex> lock(pruningMutex);
        if (depth == 0) {
            return evaluatePosition(board, gameProgress);
        }
    }

    // Transposition Table Check
    int origAlpha = alpha;
    int origBeta = beta;
    Move bestMove;
    int score;
    MoveType moveType;

    if (transpositionTable.probeEntry(board, depth, score, moveType, bestMove)) {
        std::lock_guard<std::mutex> lock(ttMutex);

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
            {
                std::lock_guard<std::mutex> lock(pruningMutex);
                pruningCount++;
            }
            return score;
        }
    }

    // Generate possible moves
    Occupant currentPlayer = maximizingPlayer ? Occupant::BLACK : Occupant::WHITE;
    std::vector<Move> possibleMoves = board.generateMoves(currentPlayer);

    if (possibleMoves.empty()) {
        return maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    }

    // Move ordering
    Move ttBestMove;
    bool hasTTMove = transpositionTable.getBestMove(board, ttBestMove);
    orderMoves(possibleMoves, board, currentPlayer, hasTTMove ? ttBestMove : Move(), depth);

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
            eval = minimax(tempBoard, depth - 1, alpha, beta, !maximizingPlayer, gameProgress);
            firstMove = false;
        }
        else {
            // Null window search (PVS)
            eval = minimax(tempBoard, depth - 1, alpha, alpha + 1, !maximizingPlayer, gameProgress);
            if (eval > alpha && eval < beta) {
                // Full re-search if null-window fails
                eval = minimax(tempBoard, depth - 1, alpha, beta, !maximizingPlayer, gameProgress);
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
            {
                std::lock_guard<std::mutex> lock(pruningMutex);
                pruningCount++;
            }
            updateKillerMove(move, depth);  // Update killer move on cutoff
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
    {
        std::lock_guard<std::mutex> lock(ttMutex);
        transpositionTable.storeEntry(board, depth, value, entryType, localBestMove);
    }

    return value;
}

AbaloneAI::AbaloneAI(int depth, int timeLimitMs, size_t ttSizeInMB)
    : maxDepth(depth), nodesEvaluated(0), timeLimit(timeLimitMs),
    timeoutOccurred(false), transpositionTable(ttSizeInMB),
    killerMoves(depth + 1) {
    pruningCount = 0;
}

std::pair<Move, int> AbaloneAI::findBestMove(Board& board, float gameProgress) {
    nodesEvaluated = 0;
    timeoutOccurred = false;
    startTime = std::chrono::high_resolution_clock::now();

    transpositionTable.incrementAge();
    killerMoves = std::vector<std::array<Move, MAX_KILLER_MOVES>>(maxDepth + 1);

    Occupant currentPlayer = board.nextToMove;
    bool maximizingPlayer = (currentPlayer == Occupant::BLACK);

    if (gameProgress == 0.0f && currentPlayer == Occupant::BLACK) {
        std::vector<Move> allMoves = board.generateMoves(currentPlayer);
        if (!allMoves.empty()) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dist(0, static_cast<int>(allMoves.size() - 1));
            int randomIndex = dist(gen);
            Move randomMove = allMoves[randomIndex];
            return std::make_pair(randomMove, 0);
        }
    }

    // Check if we're in endgame with tied scores
    int blackMarbles = 0;
    int whiteMarbles = 0;
    for (int i = 0; i < Board::NUM_CELLS; i++) {
        if (board.occupant[i] == Occupant::BLACK)
            blackMarbles++;
        else if (board.occupant[i] == Occupant::WHITE)
            whiteMarbles++;
    }

    // Calculate if scores are roughly tied
    bool scoresTied = blackMarbles == whiteMarbles;

    // Check if we're near the end of the game (high game progress)
    bool endgameNear = gameProgress >= 0.9f;

    bool closeToWin = false;
    if (maximizingPlayer) {
        closeToWin = (whiteMarbles - 1) == ENDGAME;
    }
    else {
        closeToWin = (blackMarbles - 1) == ENDGAME;
    }

    bool isLosing = false;
    if (maximizingPlayer) {
        isLosing = blackMarbles < whiteMarbles;
    }
    else {
        isLosing = whiteMarbles < blackMarbles;
    }

    bool isCloseToLosing = false;
    if (maximizingPlayer) {
        isCloseToLosing = (blackMarbles - 1) == ENDGAME;
    }
    else {
        isCloseToLosing = (whiteMarbles - 1) == ENDGAME;
    }

    std::vector<Move> possibleMoves = board.generateMoves(currentPlayer);

    if (possibleMoves.empty()) {
        return std::make_pair(Move(), 0);
    }

    Move bestTempMove;
    int bestTempScore = maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    bool foundBestMove = false;

    std::unordered_set<int> endangeredMarbles;
    for (int i = 0; i < 61; ++i) {
        if (board.occupant[i] == currentPlayer && board.isMarbleInDanger(i, currentPlayer)) {
            endangeredMarbles.insert(i);
        }
    }

    for (const auto& move : possibleMoves) {
        for (int idx : move.marbleIndices) {
            if (endangeredMarbles.count(idx)) {
                // Simulate the move
                Board tempBoard = board;
                tempBoard.applyMove(move);
                int currentScore = evaluatePosition(board, gameProgress);
                int tempScore = evaluatePosition(tempBoard, gameProgress);
                // Check if the marble is now safer
                bool stillInDanger = false;
                for (int marble : move.marbleIndices) {
                    if (tempBoard.isMarbleInDanger(marble, currentPlayer)) {
                        stillInDanger = true;
                        break;
                    }
                }
                if (!stillInDanger) {
                    // Give bonus points for defensive moves that rescue pieces
                    if (currentPlayer == Occupant::BLACK) {
                        tempScore += 500; // Defensive bonus
                    } else {
                        tempScore -= 500; // Defensive bonus (for WHITE lower is better)
                    }
                    
                    bool isBetter = (currentPlayer == Occupant::BLACK && tempScore > currentScore) || 
                                   (currentPlayer == Occupant::WHITE && tempScore < currentScore);
                                   
                    if (isBetter) {
                        std::cout << "Comparing defensive move score: " << tempScore << std::endl;
                        if (foundBestMove) {
                            bool isBest = (currentPlayer == Occupant::BLACK && tempScore > bestTempScore) || 
                                           (currentPlayer == Occupant::WHITE && tempScore < bestTempScore);
                            if (isBest) {
                                std::cout << "Found better defensive move, score: " << tempScore << std::endl;
                                bestTempScore = tempScore;
                                bestTempMove = move;
                            }
                        } else {
                            std::cout << "Found first defensive move, score: " << tempScore << std::endl;
                            bestTempScore = tempScore;
                            bestTempMove = move;
                            foundBestMove = true;
                            std::cout << "Found defensive move, score: " << tempScore << std::endl;
                        }
                    }
                }
            }
        }
    }

    if (isCloseToLosing && foundBestMove) {
        std::cout << "Close to losing: Returning best defensive move\n";
        return std::make_pair(bestTempMove, bestTempScore);
    }

    for (const auto& move : possibleMoves) {
        if (board.isPushMove(move, currentPlayer)) {
            // Evaluate the move by simulating it first
            Board tempBoard = board;
            tempBoard.applyMove(move);
            int tempScore = evaluatePosition(tempBoard, gameProgress);
            int currentScore = evaluatePosition(board, gameProgress);

            int tempBlackMarbles = 0;
            int tempWhiteMarbles = 0;
            for (int i = 0; i < Board::NUM_CELLS; i++) {
                if (tempBoard.occupant[i] == Occupant::BLACK)
                    tempBlackMarbles++;
                else if (tempBoard.occupant[i] == Occupant::WHITE)
                    tempWhiteMarbles++;
            }
            
            bool isScoringMove = false;
            // Check if the move is beneficial
            if (currentPlayer == Occupant::BLACK && tempWhiteMarbles < whiteMarbles) {
                std::cout << "Black player pushing opponent marbles off" << std::endl;
                isScoringMove = true;
            }
            else if (currentPlayer == Occupant::WHITE && tempBlackMarbles < blackMarbles) {
                std::cout << "White player pushing opponent marbles off" << std::endl;
                isScoringMove = true;
            }
            
            // Only directly select the push move if it improves position
            if (currentPlayer == Occupant::BLACK && tempScore > currentScore &&
                isScoringMove || currentPlayer == Occupant::WHITE && tempScore < currentScore &&
                isScoringMove) {
                std::cout << "Comparing push move score: " << tempScore << std::endl;

                bool isBetter = (currentPlayer == Occupant::BLACK && tempScore > bestTempScore) || 
                (currentPlayer == Occupant::WHITE && tempScore < bestTempScore);
                
                if (isBetter) {
                    bestTempScore = tempScore;
                    bestTempMove = move;
                    foundBestMove = true;
                    std::cout << "Found better push move, score: " << tempScore << std::endl;
                }
                
            }
        }
    }

    // If we found a good move (defensive or push), return it
    if (foundBestMove) {
        std::cout << "Selected best move with score: " << bestTempScore << std::endl;
        return std::make_pair(bestTempMove, bestTempScore);
    }

    // If we're in endgame with tied scores, prioritize pushing moves immediately
    if (scoresTied && endgameNear || closeToWin && endgameNear || isLosing && endgameNear) {
        if (isLosing) {
            std::cout << "Endgame with losing scores: Prioritizing push moves" << std::endl;
        } else if (closeToWin) {
            std::cout << "Endgame with winning scores: Prioritizing push moves" << std::endl;
        } else {
            std::cout << "Endgame with tied scores: Prioritizing push moves" << std::endl;
        }
        // Look for pushing moves
        for (const auto& move : possibleMoves) {
            if (board.isPushMove(move, currentPlayer)) {
                // Calculate rough score for logging purposes
                Board tempBoard = board;
                tempBoard.applyMove(move);
                int tempScore = evaluatePosition(tempBoard, gameProgress);
                int currentScore = evaluatePosition(board, gameProgress);

                int tempBlackMarbles = 0;
                int tempWhiteMarbles = 0;
                for (int i = 0; i < Board::NUM_CELLS; i++) {
                    if (tempBoard.occupant[i] == Occupant::BLACK)
                        tempBlackMarbles++;
                    else if (tempBoard.occupant[i] == Occupant::WHITE)
                        tempWhiteMarbles++;
                }

                bool isScoringMove = false;
                // Check if the move is beneficial
                if (currentPlayer == Occupant::BLACK && tempWhiteMarbles < whiteMarbles) {
                    std::cout << "Black player pushing opponent marbles off" << std::endl;
                    isScoringMove = true;
                }
                else if (currentPlayer == Occupant::WHITE && tempBlackMarbles < blackMarbles) {
                    std::cout << "White player pushing opponent marbles off" << std::endl;
                    isScoringMove = true;
                }
                
                if (isScoringMove) {
                    std::cout << "Directly selecting push move with score: " << tempScore << std::endl;
                    return std::make_pair(move, tempScore);
                }
            }
        }
    }

    std::cout << "Regular move evaluation" << std::endl;

    Move ttBestMove;
    bool hasTTMove = transpositionTable.getBestMove(board, ttBestMove);
    orderMoves(possibleMoves, board, currentPlayer, hasTTMove ? ttBestMove : Move(), maxDepth);

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
            if (timeoutOccurred) return std::make_pair(0, Move()); // Stop early

            const Move& move = possibleMoves[i];
            Board tempBoard = board;
            tempBoard.applyMove(move);
            int score = this->minimax(tempBoard, maxDepth - 1,
                std::numeric_limits<int>::min(),
                std::numeric_limits<int>::max(),
                !maximizingPlayer,
                gameProgress);
                
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
            std::lock_guard<std::mutex> lock(timeoutMutex);
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

std::pair<Move, int> AbaloneAI::findBestMoveIterativeDeepening(Board& board, int maxSearchDepth, int moveCount, int totalMoves) {
    // Clamp the maximum search depth to the object's maxDepth.
    maxSearchDepth = std::min(maxSearchDepth, this->maxDepth);

    nodesEvaluated = 0;
    timeoutOccurred = false;
    startTime = std::chrono::high_resolution_clock::now();

    Move bestMove;
    int bestScore = 0;
    bool foundMove = false;

    // Reset killer moves for each new search
    killerMoves = std::vector<std::array<Move, MAX_KILLER_MOVES>>(maxSearchDepth + 1);

    std::cout << "Move count: " << moveCount << std::endl;
    std::cout << "Total moves: " << totalMoves << std::endl;

    float gameProgress = static_cast<float>(moveCount) / totalMoves;
    gameProgress = std::min(1.0f, std::max(0.0f, gameProgress));

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
        int originalTimeLimit = timeLimit;
        timeLimit = remainingTime;

        int originalMaxDepth = maxDepth;
        maxDepth = depth;

        auto result = findBestMove(board, gameProgress);

        // Restore original time limit and max depth.
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
        auto result = findBestMove(board, gameProgress);
        bestMove = result.first;
        bestScore = result.second;
    }

    std::cout << "Transposition table usage: " << transpositionTable.getUsage() << "%" << std::endl;
    std::cout << "TT hit rate: " << transpositionTable.getHitRate() << "%" << std::endl;
    std::cout << "Pruning count: " << pruningCount << std::endl;
    std::cout << "Game progress: " << gameProgress << std::endl;

    return std::make_pair(bestMove, bestScore);
}
