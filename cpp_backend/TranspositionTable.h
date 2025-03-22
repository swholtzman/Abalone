#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include "Board.h"
#include <vector>
#include <cstdint>

// Define MoveType enum outside of the class
enum class MoveType {
    EXACT,      // Exact evaluation
    LOWERBOUND, // Alpha cutoff (score >= alpha)
    UPPERBOUND  // Beta cutoff (score <= beta)
};

// Define TTEntry completely before using it in std::vector
struct TTEntry {
    uint64_t key;       // Zobrist hash key
    int depth;          // Search depth
    int score;          // Evaluation score
    MoveType type;      // Type of node (exact, lower bound, upper bound)
    Move bestMove;      // Best move from this position
    bool isOccupied;         // Entry validity flag. An invalid entry is considered empty.
    int age;            // Age of the entry (used for replacement strategy)
    
    // Constructor
    TTEntry() : key(0), depth(0), score(0), type(MoveType::EXACT), isOccupied(false), age(0) {}
};

class TranspositionTable {
public:
    // Constructor - size in MB
    TranspositionTable(size_t sizeInMB = 64);
    
    // Clear the table
    void clearTable();
    
    // Store a position in the transposition table
    void storeEntry(const Board& board, int depth, int score, MoveType moveType, const Move& bestMove);
    
    // Probe the transposition table for a position
    bool probeEntry(const Board& board, int depth, int& score, MoveType& moveType, Move& bestMove);
    
    // Get the best move from transposition table without depth/score requirements
    bool getBestMove(const Board& board, Move& bestMove);
    
    // Get the current usage percentage of the table
    double getUsage();
    
    // Compute Zobrist hash for a given board position
    uint64_t computeHash(const Board& board);

    void incrementAge();


    double getHitRate();

private:
    // Initialize Zobrist keys
    void initZobristKeys();
    
    // Zobrist keys for each cell and piece type
    static bool s_zobristInitialized;
    static std::array<std::array<uint64_t, 3>, Board::NUM_CELLS> s_zobristKeys; // [cell][piece]
    
    // Additional key for the side to move
    uint64_t m_sideToMoveKey;
    
    // The actual transposition table
    std::vector<TTEntry> m_table;

    // Age of the entries
    int m_currentAge;

    // Counters for hit rate tracking
    size_t m_hits;
    size_t m_probes;

};

#endif // TRANSPOSITION_TABLE_H