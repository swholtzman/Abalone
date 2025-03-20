#include "Board.h"
#include <stdexcept>
#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <limits>


// TEst commit
//---------------------------------------------------------------------
// Debug macro: if DEBUG is defined, DEBUG_PRINT prints; otherwise it does nothing.
#ifdef DEBUG
#define DEBUG_PRINT(x) cout << x
#else
#define DEBUG_PRINT(x)
#endif
//---------------------------------------------------------------------

namespace std {
    class thread;
}

using namespace std;

// Helper: Convert occupant enum to a string.
static string occupantToString(Occupant occ) {
    switch (occ) {
    case Occupant::EMPTY: return "EMPTY";
    case Occupant::BLACK: return "BLACK";
    case Occupant::WHITE: return "WHITE";
    default: return "UNKNOWN";
    }
}

// Directions in (dm, dy) form, matching your doc
const array<pair<int, int>, Board::NUM_DIRECTIONS> Board::DIRECTION_OFFSETS = { {
    {-1,  0}, // W
    {+1,  0}, // E
    { 0, +1}, // NW
    {+1, +1}, // NE
    {-1, -1}, // SW
    { 0, -1}  // SE
} };

static const array<int, Board::NUM_DIRECTIONS> OPPOSITES = { 1, 0, 5, 4, 3, 2 };


bool Board::isGroupAligned(const std::vector<int>& group, int& alignedDirection) const {
    if (group.size() < 2)
        return false;

    // If only 2 marbles, check if they are adjacent in any direction
    if (group.size() == 2) {
        for (int d = 0; d < NUM_DIRECTIONS; d++) {
            if (neighbors[group[0]][d] == group[1]) {
                alignedDirection = d;
                return true;
            }
        }
        return false;
    }

    // If 3 marbles, check if they form a line in any direction
    if (group.size() == 3) {
        for (int d = 0; d < NUM_DIRECTIONS; d++) {
            int a = group[0], b = group[1], c = group[2];

            // Check if a → b and b → c are in the same direction
            if (neighbors[a][d] == b && neighbors[b][d] == c) {
                alignedDirection = d;
                return true;
            }
        }
    }

    return false;
}


bool Board::isLegalMove(const Move& m) const {
    if (m.marbleIndices.empty())
        return false;
    int d = m.direction;
    int front = getFrontCell(m.marbleIndices, d);
    int dest = neighbors[front][d];
    if (dest < 0)
        return false;

    // For inline moves, check if pushing is legal.
    if (m.isInline) {
        if (occupant[dest] != Occupant::EMPTY && occupant[dest] != occupant[front]) {
            int oppCount = 0;
            int cell = dest;
            Occupant frontOcc = occupant[front];
            while (cell >= 0 && occupant[cell] != Occupant::EMPTY && occupant[cell] != frontOcc) {
                oppCount++;
                cell = neighbors[cell][d];
            }
            if (oppCount >= (int)m.marbleIndices.size())
                return false;
            if (cell >= 0 && occupant[cell] != Occupant::EMPTY)
                return false;
        }
    }
    else {  // Side-step: every marble's target cell must be valid and empty.
        for (int idx : m.marbleIndices) {
            int target = neighbors[idx][d];
            if (target < 0 || occupant[target] != Occupant::EMPTY)
                return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------
// Modified tryMove: now uses isLegalMove instead of exception catching.
//---------------------------------------------------------------------
bool Board::tryMove(const std::vector<int>& group, int direction, Move& move) const {
    move.marbleIndices = group;
    move.direction = direction;
    if (group.size() == 1) {
        move.isInline = false;
    }
    else {
        int alignedDir;
        if (isGroupAligned(group, alignedDir)) {
            move.isInline = (direction == alignedDir || direction == OPPOSITES[alignedDir]);
        }
        else {
            move.isInline = false;
        }
    }
    // Simply check legality without invoking try-catch.
    if (!isLegalMove(move)) {
        return false;
    }
    return true;
}

#include <thread>
#include <mutex>
#include <set>
#include <vector>
#include <iostream>

#include <thread>
#include <mutex>
#include <set>
#include <vector>
#include <iostream>

std::mutex groupMutex;  // Mutex to ensure thread safety when modifying shared data

static long long packCoord(int m, int y) {
    return (static_cast<long long>(m) << 32) ^ (static_cast<long long>(y) & 0xffffffff);
}





// ========================== Group Detection Functions ========================== //

// ========================== Group Detection Functions ========================== //



std::set<std::vector<int>> Board::generateGroups(Occupant side) const {
    std::set<std::vector<int>> groups;
    const std::vector<std::pair<int, int>>* targetList = nullptr;

    // Select the target list based on occupant color
    if (side == Occupant::BLACK) {
        targetList = &blackOccupantsCoords;
    }
    else if (side == Occupant::WHITE) {
        targetList = &whiteOccupantsCoords;
    }

    for (const auto& coordinate : *targetList) {
        long long key = packCoord(coordinate.first, coordinate.second);
        auto it = s_coordToIndex.find(key);
        if (it == s_coordToIndex.end()) continue;  // Skip if coordinate is invalid

        int idx = it->second;
        groups.insert({ idx });  // Add single-marble group

        for (int i = 1; i <= 3; i++) {  // Only iterate over necessary directions
            int first_neighbour_index = neighbors[idx][i];

            // Ensure first neighbor is valid and belongs to the same player
            if (first_neighbour_index == -1 || occupant[first_neighbour_index] != side) continue;

            std::vector<int> twoMarbleGroup = { idx, first_neighbour_index };
            std::sort(twoMarbleGroup.begin(), twoMarbleGroup.end());
            groups.insert(twoMarbleGroup);  // Insert two-marble group

            int second_neighbour_index = neighbors[first_neighbour_index][i];

            if (second_neighbour_index == -1 || occupant[second_neighbour_index] != side) continue;

            std::vector<int> threeMarbleGroup = { idx, first_neighbour_index, second_neighbour_index };
            std::sort(threeMarbleGroup.begin(), threeMarbleGroup.end());
            groups.insert(threeMarbleGroup);  // Insert three-marble group
        }
    }



    return groups;
}







/**
 * @brief Generates all legal moves for a given side.
 *
 * This function replaces `dfsGroup` with `generateParallelGroups`
 * for improved performance using multi-threading.
 *
 * @param side The player (Black or White) for whom to generate moves.
 * @return A list of valid moves.
 */
std::vector<Move> Board::generateMoves(Occupant side) const {
    std::vector<Move> moves;  // Stores all valid moves

    // Generate unique groups using the multi-threaded approach
    std::set<std::vector<int>> candidateGroups = generateGroups(side);


    // Iterate over each group and attempt moves in all directions
    for (const auto& group : candidateGroups) {
        for (int d = 0; d < NUM_DIRECTIONS; d++) {


            Move candidateMove;

            if (tryMove(group, d, candidateMove)) { // Validate and apply move logic
                moves.push_back(candidateMove);
            }
        }
    }

    return moves;
}



void Board::applyMove(const Move& m) {
    if (m.marbleIndices.empty()) {
        throw runtime_error("No marbles in move.");
    }
    int d = m.direction;
    static const char* DIRS[] = { "W", "E", "NW", "NE", "SW", "SE" };
    DEBUG_PRINT("Applying move: "
        << (occupant[m.marbleIndices[0]] == Occupant::BLACK ? "b" : "w")
        << ", group (size " << m.marbleIndices.size() << "): ");
    for (int idx : m.marbleIndices)
        DEBUG_PRINT(indexToNotation(idx) << " ");
    DEBUG_PRINT(", direction: " << d << " (" << DIRS[d] << ")"
        << (m.isInline ? " [inline]" : " [side-step]") << "\n");
    if (m.isInline) {
        int front = getFrontCell(m.marbleIndices, m.direction);
        int dest = neighbors[front][d];
        DEBUG_PRINT("  Front cell: " << indexToNotation(front)
            << ", destination: " << (dest >= 0 ? indexToNotation(dest) : "off-board"));
        if (dest >= 0) {
            DEBUG_PRINT(" (occupant: " << occupantToString(occupant[dest]) << ")");
        }
        DEBUG_PRINT("\n");

        if (dest >= 0 && occupant[dest] != Occupant::EMPTY && occupant[dest] != occupant[front]) {
            int oppCount = 0;
            int cell = dest;
            while (cell >= 0 && occupant[cell] != Occupant::EMPTY &&
                occupant[cell] != occupant[front]) {
                oppCount++;
                cell = neighbors[cell][d];
            }
            DEBUG_PRINT("  Push detection: oppCount = " << oppCount);
            DEBUG_PRINT(", final cell after push loop = " << (cell >= 0 ? indexToNotation(cell) : "off-board"));
            if (cell >= 0)
                DEBUG_PRINT(" (occupant: " << occupantToString(occupant[cell]) << ")");
            DEBUG_PRINT("\n");
            if (oppCount >= m.marbleIndices.size()) {
                throw runtime_error("Illegal move: cannot push, opponent group too large.");
            }
            if (cell >= 0 && occupant[cell] != Occupant::EMPTY) {
                throw runtime_error("Illegal move: push blocked, destination not empty.");
            }
            DEBUG_PRINT("  Push detected: pushing " << oppCount
                << " opponent marble" << (oppCount > 1 ? "s" : "") << ".\n");
            vector<int> chain;
            cell = dest;
            for (int i = 0; i < oppCount; i++) {
                chain.push_back(cell);
                cell = neighbors[cell][d];
            }
            for (int i = chain.size() - 1; i >= 0; i--) {
                int from = chain[i];
                int to = (i == chain.size() - 1) ? cell : chain[i + 1];
                if (to < 0) {
                    //TODO:Hello
                    occupant[from] = Occupant::EMPTY;
                    updateOccupantCoordinates(from, -1, occupant[from]); // -1 means remove only
                    DEBUG_PRINT("    Marble at " << indexToNotation(from)
                        << " pushed off-board.\n");
                }
                else {
                    if (occupant[to] != Occupant::EMPTY)
                        throw runtime_error("Illegal move: push blocked while moving opponent marbles.");
                    //TODO:Hello
                    occupant[to] = occupant[from];
                    updateOccupantCoordinates(from, to, occupant[to]);
                    occupant[from] = Occupant::EMPTY;
                    DEBUG_PRINT("    Marble at " << indexToNotation(from)
                        << " moved to " << indexToNotation(to) << ".\n");
                }
            }
        }

        // Now, we need to move our own marbles.
        // Sort the moving group by dot-product with the move offset so that the marble furthest in the direction is last.
        auto offset = DIRECTION_OFFSETS[d];
        vector<int> sortedGroup = m.marbleIndices;
        sort(sortedGroup.begin(), sortedGroup.end(), [&](int a, int b) {
            auto ca = s_indexToCoord[a];
            auto cb = s_indexToCoord[b];
            int scoreA = offset.first * ca.first + offset.second * ca.second;
            int scoreB = offset.first * cb.first + offset.second * cb.second;
            return scoreA < scoreB; // lower scores come first
            });

        // Move our own marbles in reverse order.
        for (auto it = sortedGroup.rbegin(); it != sortedGroup.rend(); ++it) {
            int idx = *it;
            int target = neighbors[idx][d];
            DEBUG_PRINT("  Moving " << indexToNotation(idx) << " to "
                << (target >= 0 ? indexToNotation(target) : "off-board") << "\n");
            if (target < 0) {
                throw runtime_error("Illegal move: marble would move off-board.");
            }
            if (occupant[target] != Occupant::EMPTY) {
                throw runtime_error("Illegal move: destination cell is not empty for inline move.");
            }
            //TODO: HELLO


            occupant[target] = occupant[idx];
            updateOccupantCoordinates(idx, target, occupant[target]);
            occupant[idx] = Occupant::EMPTY;
        }
    }
    else {
        for (int idx : m.marbleIndices) {
            int target = neighbors[idx][d];
            DEBUG_PRINT("  Side-stepping " << indexToNotation(idx) << " to "
                << (target >= 0 ? indexToNotation(target) : "off-board") << "\n");
            if (target < 0) {
                throw runtime_error("Illegal move: side-step moves off-board.");
            }
            if (occupant[target] != Occupant::EMPTY) {
                throw runtime_error("Illegal move: destination cell is not empty for side-step.");
            }
            //TODO: HELLO
            occupant[target] = occupant[idx];
            updateOccupantCoordinates(idx, target, occupant[target]);
            occupant[idx] = Occupant::EMPTY;
        }
    }
}

/**
 * Helper lambda: record a change to occupant[i], so we can undo it later.
 *
 * Usage:
 *    recordChange(i, newOcc);
 * This appends a CellChange to delta.changes, then writes newOcc into occupant[i].
 */
inline void recordChange(std::array<Occupant, Board::NUM_CELLS>& occupantArray,
    MoveDelta& delta, int i, Occupant newOcc)
{
    MoveDelta::CellChange change;
    change.index = i;
    change.oldOcc = occupantArray[i];
    change.newOcc = newOcc;
    delta.changes.push_back(change);

    // Actually update occupant
    occupantArray[i] = newOcc;
}


MoveDelta Board::applyMoveInPlace(const Move& m)
{
    MoveDelta delta;
    if (m.marbleIndices.empty()) {
        throw std::runtime_error("No marbles in move.");
    }

    int d = m.direction;

    // We follow the same logic you have in applyMove:
    // 1) If it's inline, push opponents if necessary.
    // 2) Move own marbles in correct order.
    // 3) If side-step, just move each marble to the side.

    // Figure out whether it's inline or sidestep from Move.isInline
    // We replicate your push logic, but call recordChange() each time occupant[] is modified.

    if (m.isInline) {
        // 1) Identify the front marble
        int frontIdx = getFrontCell(m.marbleIndices, d);
        int dest = neighbors[frontIdx][d];

        // If occupant[dest] is occupied by the opposite color, handle pushing
        if (dest >= 0 &&
            occupant[dest] != Occupant::EMPTY &&
            occupant[dest] != occupant[frontIdx])
        {
            // Count how many opponent marbles in a row
            int oppCount = 0;
            int cell = dest;
            Occupant frontOcc = occupant[frontIdx];
            while (cell >= 0 && occupant[cell] != Occupant::EMPTY && occupant[cell] != frontOcc) {
                oppCount++;
                cell = neighbors[cell][d];
            }

            // If oppCount >= group size => invalid push
            if (oppCount >= (int)m.marbleIndices.size()) {
                throw std::runtime_error("Illegal move: cannot push, opponent group too large.");
            }

            // If the final cell is on board and not empty => blocked
            if (cell >= 0 && occupant[cell] != Occupant::EMPTY) {
                throw std::runtime_error("Illegal move: push blocked, destination not empty.");
            }

            // We push the chain of opponent marbles
            std::vector<int> chain;
            cell = dest;
            for (int i = 0; i < oppCount; i++) {
                chain.push_back(cell);
                cell = neighbors[cell][d]; // move further along
            }

            // Move them from last to first
            for (int i = (int)chain.size() - 1; i >= 0; --i) {
                int from = chain[i];
                int to = (i == (int)chain.size() - 1) ? cell : chain[i + 1];

                // If to < 0, the marble is pushed off the board
                if (to < 0) {
                    // record occupant[from] -> EMPTY
                    recordChange(occupant, delta, from, Occupant::EMPTY);
                }
                else {
                    // Must be empty
                    if (occupant[to] != Occupant::EMPTY) {
                        throw std::runtime_error("Illegal move: push blocked while moving opponent marbles.");
                    }
                    // Move occupant[from] -> occupant[to], occupant[from] = EMPTY
                    Occupant fromOcc = occupant[from];
                    recordChange(occupant, delta, to, fromOcc);
                    recordChange(occupant, delta, from, Occupant::EMPTY);
                }
            }
        }

        // 2) Move our own marbles in reverse order to avoid overwriting
        // Sort the group by dot-product with direction
        auto offset = DIRECTION_OFFSETS[d];
        std::vector<int> sortedGroup = m.marbleIndices;
        std::sort(sortedGroup.begin(), sortedGroup.end(),
            [&](int a, int b) {
                auto ca = s_indexToCoord[a];
                auto cb = s_indexToCoord[b];
                int scoreA = offset.first * ca.first + offset.second * ca.second;
                int scoreB = offset.first * cb.first + offset.second * cb.second;
                return scoreA < scoreB;
            });

        // Move from last to first
        for (auto it = sortedGroup.rbegin(); it != sortedGroup.rend(); ++it) {
            int idx = *it;
            int target = neighbors[idx][d];
            if (target < 0) {
                throw std::runtime_error("Illegal move: marble would move off-board.");
            }
            if (occupant[target] != Occupant::EMPTY) {
                throw std::runtime_error("Illegal move: destination cell is not empty for inline move.");
            }
            // occupant[target] = occupant[idx], occupant[idx] = EMPTY
            Occupant fromOcc = occupant[idx];
            recordChange(occupant, delta, target, fromOcc);
            recordChange(occupant, delta, idx, Occupant::EMPTY);
        }
    }
    else {
        // Side-step
        // For each marble in m.marbleIndices, move it sideways one step
        for (int idx : m.marbleIndices) {
            int target = neighbors[idx][d];
            if (target < 0) {
                throw std::runtime_error("Illegal move: side-step moves off-board.");
            }
            if (occupant[target] != Occupant::EMPTY) {
                throw std::runtime_error("Illegal move: destination cell is not empty for side-step.");
            }
            // occupant[target] = occupant[idx], occupant[idx] = EMPTY
            Occupant fromOcc = occupant[idx];
            recordChange(occupant, delta, target, fromOcc);
            recordChange(occupant, delta, idx, Occupant::EMPTY);
        }
    }

    // If you rely on blackOccupantsCoords / whiteOccupantsCoords for your AI or logging,
    // you could update them here OR skip it if you plan to do a fresh occupantCoordinates
    // update only after the full search. For instance:
    // updateOccupantCoordinates(); // but it's slower if called every time

    return delta;
}

void Board::undoMove(const MoveDelta& delta)
{
    // Revert occupant changes in reverse order of application
    for (auto it = delta.changes.rbegin(); it != delta.changes.rend(); ++it) {
        occupant[it->index] = it->oldOcc;
    }

    // If you update occupantCoords above, you'd have to revert them too.
    // Otherwise, you can skip if occupantCoords are only updated at top level.
    // Example:
    // updateOccupantCoordinates();
}


int Board::getFrontCell(const vector<int>& group, int direction) const {
    auto offset = DIRECTION_OFFSETS[direction];
    int bestIdx = group.front();
    int bestScore = numeric_limits<int>::min();
    for (int idx : group) {
        auto coord = s_indexToCoord[idx];  // (m, y)
        int score = offset.first * coord.first + offset.second * coord.second;
        if (score > bestScore) {
            bestScore = score;
            bestIdx = idx;
        }
    }
    return bestIdx;
}

string Board::moveToNotation(const Move& m, Occupant side) {
    string notation;
    char teamChar = (side == Occupant::BLACK ? 'b' : 'w');
    vector<string> cellNotations;
    for (int idx : m.marbleIndices) {
        cellNotations.push_back(indexToNotation(idx));
    }
    sort(cellNotations.begin(), cellNotations.end(), greater<string>());
    notation = "(";
    notation.push_back(teamChar);
    notation += ", ";
    for (size_t i = 0; i < cellNotations.size(); i++) {
        if (i > 0)
            notation += ", ";
        notation += cellNotations[i];
    }
    notation += ") ";
    notation += (m.isInline ? "i" : "s");
    notation += " → ";
    static const char* DIRS[] = { "W", "E", "NW", "NE", "SW", "SE" };
    notation += DIRS[m.direction];
    return notation;
}

string Board::toBoardString() const {
    string result;
    bool first = true;
    for (int i = 0; i < NUM_CELLS; i++) {
        if (occupant[i] == Occupant::BLACK || occupant[i] == Occupant::WHITE) {
            if (!first)
                result += ",";
            result += indexToNotation(i);
            result += (occupant[i] == Occupant::BLACK ? "b" : "w");
            first = false;
        }
    }
    return result;
}

string Board::indexToNotation(int idx) {
    auto [m, y] = s_indexToCoord[idx];
    char rowLetter = char('A' + (y - 1));
    string notation;
    notation.push_back(rowLetter);
    notation += to_string(m);
    return notation;
}

//========================== Hardcoded Layouts ==========================//

void Board::initStandardLayout() {
    occupant.fill(Occupant::EMPTY);
    vector<string> blackPositions = {
        "A4", "A5",
        "B4", "B5", "B6",
        "C4", "C5", "C6", "C7",
        "D5", "D6", "D7",
        "E5", "E6"
    };
    for (auto& cell : blackPositions) {
        setOccupant(cell, Occupant::BLACK);
    }
    vector<string> whitePositions = {
        "E4", "F4", "F5", "F6", "F7",
        "G3", "G4", "G5", "G6", "G7",
        "H4", "H5", "H6",
        "I5"
    };
    for (auto& cell : whitePositions) {
        setOccupant(cell, Occupant::WHITE);
    }

    updateOccupantCoordinates();
}

void Board::initBelgianDaisyLayout() {
    occupant.fill(Occupant::EMPTY);
    vector<string> blackPositions = {
        "C5","C6","D4","D7","E4","E7","F4","F7","G5","G6"
    };
    for (auto& cell : blackPositions) {
        setOccupant(cell, Occupant::BLACK);
    }
    vector<string> whitePositions = {
        "C4","D3","E3","F3","G4","G7","D8","E8","F8","G8"
    };
    for (auto& cell : whitePositions) {
        setOccupant(cell, Occupant::WHITE);
    }
    updateOccupantCoordinates();
}

void Board::initGermanDaisyLayout() {
    occupant.fill(Occupant::EMPTY);
    vector<string> blackPositions = {
        "B4","C4","D5","E5","F5","G5","H6"
    };
    for (auto& cell : blackPositions) {
        setOccupant(cell, Occupant::BLACK);
    }
    vector<string> whitePositions = {
        "B5","C5","D4","E4","F4","G4","H5"
    };
    for (auto& cell : whitePositions) {
        setOccupant(cell, Occupant::WHITE);
    }
    updateOccupantCoordinates();
}

//========================== Loading from Input File ==========================//

bool Board::loadFromInputFile(const string& filename) {
    occupant.fill(Occupant::EMPTY);
    ifstream fin(filename);
    if (!fin.is_open()) {
        cerr << "Error: could not open file: " << filename << "\n";
        return false;
    }
    string line;
    if (!getline(fin, line)) {
        cerr << "Error: file is missing the first line.\n";
        return false;
    }
    if (line.size() < 1) {
        cerr << "Error: first line is empty.\n";
        return false;
    }
    char nextColorChar = line[0];
    if (nextColorChar == 'b' || nextColorChar == 'B') {
        nextToMove = Occupant::BLACK;
    }
    else if (nextColorChar == 'w' || nextColorChar == 'W') {
        nextToMove = Occupant::WHITE;
    }
    else {
        cerr << "Error: first line must be 'b' or 'w'. Found: " << line << "\n";
        return false;
    }
    if (!getline(fin, line)) {
        cerr << "Error: file is missing the second line.\n";
        return false;
    }
    stringstream ss(line);
    string token;
    while (getline(ss, token, ',')) {
        if (token.empty()) continue;
        char c = token.back();
        Occupant who;
        if (c == 'b' || c == 'B') who = Occupant::BLACK;
        else if (c == 'w' || c == 'W') who = Occupant::WHITE;
        else {
            cerr << "Warning: token '" << token << "' does not end in b/w. Skipping.\n";
            continue;
        }
        token.pop_back();
        setOccupant(token, who);
    }

    updateOccupantCoordinates();
    fin.close();
    return true;
}


void Board::updateOccupantCoordinates() {
    blackOccupantsCoords.clear();
    whiteOccupantsCoords.clear();

    for (int i = 0; i < NUM_CELLS; i++) {
        if (occupant[i] == Occupant::BLACK) {
            blackOccupantsCoords.push_back(s_indexToCoord[i]);
        }
        else if (occupant[i] == Occupant::WHITE) {
            whiteOccupantsCoords.push_back(s_indexToCoord[i]);
        }
    }

    // Sort the lists to maintain order from A1 to A5, B1 to B6, etc.
    auto sortingLambda = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        return (a.second == b.second) ? (a.first < b.first) : (a.second < b.second);
        };

    std::sort(blackOccupantsCoords.begin(), blackOccupantsCoords.end(), sortingLambda);
    std::sort(whiteOccupantsCoords.begin(), whiteOccupantsCoords.end(), sortingLambda);
}


void Board::updateOccupantCoordinates(int oldIndex, int newIndex, Occupant occupantType) {
    std::vector<std::pair<int, int>>* targetList = nullptr;

    if (occupantType == Occupant::BLACK) {
        targetList = &blackOccupantsCoords;
    }
    else if (occupantType == Occupant::WHITE) {
        targetList = &whiteOccupantsCoords;
    }

    if (targetList) {
        // Remove the old coordinate if valid
        if (oldIndex >= 0) {
            std::pair<int, int> oldCoord = s_indexToCoord[oldIndex];
            auto it = std::lower_bound(targetList->begin(), targetList->end(), oldCoord,
                [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                    return (a.second == b.second) ? (a.first < b.first) : (a.second < b.second);
                });
            if (it != targetList->end() && *it == oldCoord) {
                targetList->erase(it);
            }
        }

        // Insert the new coordinate in the correct position
        if (newIndex >= 0) {
            std::pair<int, int> newCoord = s_indexToCoord[newIndex];
            auto it = std::lower_bound(targetList->begin(), targetList->end(), newCoord,
                [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                    return (a.second == b.second) ? (a.first < b.first) : (a.second < b.second);
                });

            // Ensure we are not inserting duplicates
            if (it == targetList->end() || *it != newCoord) {
                targetList->insert(it, newCoord);  // Insert in sorted position
            }
        }
    }
}




void Board::setOccupant(const string& notation, Occupant who, bool updateCoords) {
    int idx = notationToIndex(notation);
    if (idx >= 0) {
        occupant[idx] = who;

        // Only update coordinates if updateCoords is true
        if (updateCoords) {
            updateOccupantCoordinates(idx, -1, who);  // -1 means we're just adding, no removal needed
        }
    }
    else {
        cerr << "Warning: invalid cell notation '" << notation << "'\n";
    }
}




//========================== Mapping, Neighbors, etc. ==========================//

bool Board::s_mappingInitialized = false;
unordered_map<long long, int> Board::s_coordToIndex;
array<pair<int, int>, Board::NUM_CELLS> Board::s_indexToCoord;

// static long long packCoord(int m, int y) {
//     return (static_cast<long long>(m) << 32) ^ (static_cast<long long>(y) & 0xffffffff);
// }

void Board::initMapping() {
    if (s_mappingInitialized) return;
    s_mappingInitialized = true;
    s_coordToIndex.clear();
    int idx = 0;
    for (int y = 1; y <= 9; ++y) {
        for (int m = 1; m <= 9; ++m) {
            bool validCell = false;
            switch (y) {
            case 1: validCell = (m >= 1 && m <= 5); break;
            case 2: validCell = (m >= 1 && m <= 6); break;
            case 3: validCell = (m >= 1 && m <= 7); break;
            case 4: validCell = (m >= 1 && m <= 8); break;
            case 5: validCell = (m >= 1 && m <= 9); break;
            case 6: validCell = (m >= 2 && m <= 9); break;
            case 7: validCell = (m >= 3 && m <= 9); break;
            case 8: validCell = (m >= 4 && m <= 9); break;
            case 9: validCell = (m >= 5 && m <= 9); break;
            }
            if (validCell) {
                long long key = packCoord(m, y);
                s_coordToIndex[key] = idx;
                s_indexToCoord[idx] = { m, y };
                ++idx;
            }
        }
    }
    if (idx != NUM_CELLS) {
        throw runtime_error("Did not fill exactly 61 cells! Check your loops!");
    }
}

Board::Board() {
    initMapping();
    occupant.fill(Occupant::EMPTY);
    initNeighbors();
}

void Board::initNeighbors() {
    for (int i = 0; i < NUM_CELLS; ++i) {
        int m = s_indexToCoord[i].first;
        int y = s_indexToCoord[i].second;
        for (int d = 0; d < NUM_DIRECTIONS; ++d) {
            int dm = DIRECTION_OFFSETS[d].first;
            int dy = DIRECTION_OFFSETS[d].second;
            int nm = m + dm;
            int ny = y + dy;
            long long nkey = packCoord(nm, ny);
            auto it = s_coordToIndex.find(nkey);
            if (it == s_coordToIndex.end()) {
                neighbors[i][d] = -1;
            }
            else {
                neighbors[i][d] = it->second;
            }
        }
    }
}

int Board::notationToIndex(const string& notation) {
    if (notation.size() < 2 || notation.size() > 3) {
        return -1;
    }
    char letter = toupper(notation[0]);
    int y = (letter - 'A') + 1;
    if (y < 1 || y > 9) {
        return -1;
    }
    int m = 0;
    try {
        m = stoi(notation.substr(1));
    }
    catch (...) {
        return -1;
    }
    if (m < 1 || m > 9) {
        return -1;
    }
    long long key = packCoord(m, y);
    auto it = s_coordToIndex.find(key);
    if (it == s_coordToIndex.end()) {
        return -1;
    }
    return it->second;
}
