#include "TranspositionTable.h"
#include <random>
#include <chrono>

// Zobrist key initialization
bool TranspositionTable::s_zobristInitialized = false;
std::array<std::array<uint64_t, 3>, Board::NUM_CELLS> TranspositionTable::s_zobristKeys;

TranspositionTable::TranspositionTable(size_t sizeInMB) {
    initZobristKeys();
    size_t entryCount = (sizeInMB * 1024 * 1024) / sizeof(TTEntry);
    m_table.resize(entryCount);
    clearTable();
}

void TranspositionTable::initZobristKeys() {
    // Initialize only once
    if (s_zobristInitialized) return;
    
    // Initialize with random values using a good random number generator
    std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<uint64_t> dist;
    
    for (int i = 0; i < Board::NUM_CELLS; ++i) {
        for (int j = 0; j < 3; ++j) { // 3 = EMPTY, BLACK, WHITE
            s_zobristKeys[i][j] = dist(rng);
        }
    }
    
    // Additional key for the side to move
    m_sideToMoveKey = dist(rng);
    
    s_zobristInitialized = true;
}

// Clear the table
void TranspositionTable::clearTable() {
    for (auto& entry : m_table) {
        entry.isOccupied = false;
    }
}

// Compute Zobrist hash for a given board position
uint64_t TranspositionTable::computeHash(const Board& board) {
    uint64_t hash = 0;
    
    // Hash the occupants
    for (int i = 0; i < Board::NUM_CELLS; ++i) {
        Occupant occ = board.getOccupant(i);
        if (occ != Occupant::EMPTY) {
            int occIndex = (occ == Occupant::BLACK) ? 1 : 2;
            hash ^= s_zobristKeys[i][occIndex];
        }
    }
    
    // Hash the side to move
    if (board.nextToMove == Occupant::WHITE) {
        hash ^= m_sideToMoveKey;
    }
    
    return hash;
}

// Store a position in the transposition table
void TranspositionTable::storeEntry(const Board& board, int depth, int score, MoveType moveType, const Move& bestMove) {
    uint64_t hash = computeHash(board);
    size_t index = hash % m_table.size();
    
    TTEntry& entry = m_table[index];
    
    // Only overwrite if current entry is occupied, has lower depth, or this is the same position
    if (!entry.isOccupied || depth >= entry.depth || entry.key == hash) {
        entry.key = hash;
        entry.depth = depth;
        entry.score = score;
        entry.type = moveType;
        entry.bestMove = bestMove;
        entry.isOccupied = true;
    }
}

// Probe the transposition table for a position
bool TranspositionTable::probeEntry(const Board& board, int depth, int& score, MoveType& moveType, Move& bestMove) {
    uint64_t hash = computeHash(board);
    size_t index = hash % m_table.size();
    
    TTEntry& entry = m_table[index];
    
    // Checks if the entry is occupied AND if its Zobrist key matches the current position's hash
    if (entry.isOccupied && entry.key == hash && entry.depth >= depth) {
        score = entry.score;
        moveType = entry.type;
        bestMove = entry.bestMove;
        return true;
    }
    
    return false;
}

// Get the best move from transposition table without depth/score requirements
bool TranspositionTable::getBestMove(const Board& board, Move& bestMove) {
    uint64_t hash = computeHash(board);
    size_t index = hash % m_table.size();
    
    TTEntry& entry = m_table[index];
    
    // Checks if the entry is occupied AND if its Zobrist key matches the current position's hash
    if (entry.isOccupied && entry.key == hash) {
        bestMove = entry.bestMove;
        return true;
    }
    
    return false;
}

// Get the current usage percentage of the table
double TranspositionTable::getUsage() {
    size_t usedEntries = 0;
    
    for (const auto& entry : m_table) {
        if (entry.isOccupied) {
            usedEntries++;
        }
    }
    
    return (double)usedEntries / m_table.size() * 100.0;
}