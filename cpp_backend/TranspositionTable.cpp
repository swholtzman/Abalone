#include "TranspositionTable.h"
#include <random>
#include <chrono>
#include <iostream>

// Zobrist key initialization
bool TranspositionTable::s_zobristInitialized = false;
std::array<std::array<uint64_t, 3>, Board::NUM_CELLS> TranspositionTable::s_zobristKeys;

TranspositionTable::TranspositionTable(size_t sizeInMB) {
    initZobristKeys();
    size_t entryCount = (sizeInMB * 1024 * 1024) / sizeof(TTEntry);
    m_table.resize(entryCount);
    m_currentAge = 0;
    m_hits = 0;
    m_probes = 0;
    clearTable();
}

void TranspositionTable::incrementAge() {
    m_currentAge++;
}

double TranspositionTable::getHitRate() {
    return (m_probes > 0) ? ((double)m_hits / m_probes * 100.0) : 0.0;
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
    std::memset(m_table.data(), 0, m_table.size() * sizeof(TTEntry));
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
    
    // Always replace with the following exceptions:
    // 1. If it's the same position but we have a deeper search stored
    // 2. If the existing entry is from the current search and has higher depth
    bool shouldReplace = true;
    
    if (entry.isOccupied && entry.key == hash) {
        // Same position - check depth
        if (entry.depth > depth && entry.age == m_currentAge) {
            // Keep existing entry - it's deeper and from current search
            shouldReplace = false;
        }
    }
    
    if (shouldReplace) {
        entry.key = hash;
        entry.depth = depth;
        entry.score = score;
        entry.type = moveType;
        entry.bestMove = bestMove;
        entry.isOccupied = true;
        entry.age = m_currentAge;  // Update age to current search
    }
}

// Probe the transposition table for a position
bool TranspositionTable::probeEntry(const Board& board, int depth, int& score, MoveType& moveType, Move& bestMove) {
    uint64_t hash = computeHash(board);
    size_t index = hash % m_table.size();

    m_probes++;  // Increment probe counter
    
    TTEntry& entry = m_table[index];
    
    // Checks if the entry is valid and matches our position
    if (entry.isOccupied && entry.key == hash) {
        // We found a matching position
        if (entry.depth >= depth) {
            m_hits++;  // Increment hit counter
            score = entry.score;
            moveType = entry.type;
            bestMove = entry.bestMove;
            return true;
        }
        
        // Entry is too shallow but we can still use the move
        bestMove = entry.bestMove;
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