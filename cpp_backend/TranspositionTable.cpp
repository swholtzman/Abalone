#include "TranspositionTable.h"
#include <random>
#include <chrono>
#include <iostream>
#include <cstring>
#include <cstring>
#include <fstream>

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

void TranspositionTable::saveZobristKeysTxt(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "[Zobrist] Failed to open file for writing: " << filename << "\n";
        return;
    }

    for (int i = 0; i < Board::NUM_CELLS; ++i) {
        for (int j = 0; j < 3; ++j) {
            outFile << s_zobristKeys[i][j] << " ";
        }
        outFile << "\n";
    }

    outFile << m_sideToMoveKey << "\n";

    std::cout << "[Zobrist] Keys saved to " << filename << "\n";
}


bool TranspositionTable::loadZobristKeysTxt(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile) {
        std::cerr << "[Zobrist] Failed to open file for reading: " << filename << "\n";
        return false;
    }

    for (int i = 0; i < Board::NUM_CELLS; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (!(inFile >> s_zobristKeys[i][j])) {
                std::cerr << "[Zobrist] Failed to read key at [" << i << "][" << j << "]\n";
                return false;
            }
        }
    }

    if (!(inFile >> m_sideToMoveKey)) {
        std::cerr << "[Zobrist] Failed to read m_sideToMoveKey\n";
        return false;
    }

    std::cout << "[Zobrist] Keys loaded from " << filename << "\n";
    return true;
}


void TranspositionTable::initZobristKeys() {
    if (s_zobristInitialized) return;

    const std::string zobristFile = "zobrist_keys.txt";

    if (loadZobristKeysTxt(zobristFile)) {
        // Loaded from file
    } else {
        std::cout << "[Zobrist] Generating new Zobrist keys...\n";

        std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<uint64_t> dist;

        for (int i = 0; i < Board::NUM_CELLS; ++i) {
            for (int j = 0; j < 3; ++j) {
                s_zobristKeys[i][j] = dist(rng);
            }
        }

        m_sideToMoveKey = dist(rng);

        saveZobristKeysTxt(zobristFile);
    }

    s_zobristInitialized = true;
}

// Clear the table
void TranspositionTable::clearTable() {
    for (auto& entry : m_table) {
        entry = TTEntry();
    }
}


bool TranspositionTable::loadTableFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile) {
        std::cerr << "[TT] Failed to open file for reading: " << filename << "\n";
        return false;
    }

    clearTable();  // Start fresh

    uint64_t key;
    int depth, score, typeInt, age;
    std::string moveStr;

    while (inFile >> key >> depth >> score >> typeInt >> moveStr >> age) {
        TTEntry entry;
        entry.key = key;
        entry.depth = depth;
        entry.score = score;
        entry.type = static_cast<MoveType>(typeInt);
        entry.bestMove = Move::deserialize(moveStr);  // You'll need to implement this
        entry.age = age;
        entry.isOccupied = true;

        size_t index = key % m_table.size();
        m_table[index] = entry;
    }

    std::cout << "[TT] Transposition table loaded from " << filename << "\n";
    return true;
}

void TranspositionTable::saveTableToFile(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "[TT] Failed to open file for writing: " << filename << "\n";
        return;
    }

    for (const auto& entry : m_table) {
        if (!entry.isOccupied) continue;

        outFile << entry.key << " "
                << entry.depth << " "
                << entry.score << " "
                << static_cast<int>(entry.type) << " "
                << entry.bestMove.serialize() << " "  // You'll need to implement this
                << entry.age << "\n";
    }

    std::cout << "[TT] Transposition table saved to " << filename << "\n";
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
        // Same position
        if (entry.age == m_currentAge) {
            // Same search - prefer deeper searches or exact nodes
            if (entry.depth > depth && entry.type == MoveType::EXACT) {
                shouldReplace = false;
            }
            else if (entry.depth == depth) {
                // Equal depth, prefer more accurate node types
                if (entry.type == MoveType::EXACT && moveType != MoveType::EXACT) {
                    shouldReplace = false;
                }
            }
        }
    }

    // For entries that are almost done with their search, always keep them
    if (entry.isOccupied && entry.key == hash &&
        entry.depth >= depth + 3 && entry.age == m_currentAge - 1) {
        shouldReplace = false;
    }

    if (shouldReplace) {
        entry.key = hash;
        entry.depth = depth;
        entry.score = score;
        entry.type = moveType;
        entry.bestMove = bestMove;
        entry.isOccupied = true;
        entry.age = m_currentAge;
    }
    else if (entry.key == hash && entry.bestMove.marbleIndices.empty() && !bestMove.marbleIndices.empty()) {
        // Always update the best move if we didn't have one
        entry.bestMove = bestMove;
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