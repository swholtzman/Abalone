#ifndef ABALONE_BOARD_H
#define ABALONE_BOARD_H

#include <array>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <algorithm>
#include <unordered_set>

//------------------------------------------------------------------------------
// Occupant and Move Structures
//------------------------------------------------------------------------------

struct VectorHash;

// Simple occupant type: empty, black, or white.
enum class Occupant {
    EMPTY = 0,
    BLACK,
    WHITE
};

// Structure to represent a move.
// 'marbleIndices' holds the indices of the marbles to be moved.
// 'direction' is an integer from 0 to 5 corresponding to a movement direction.
// 'isInline' indicates whether the move is inline (true) or a sidestep (false).
// 'pushCount' can store how many opponent marbles are pushed.
struct Move {
    std::vector<int> marbleIndices;
    int direction;
    bool isInline;
    int pushCount;

    bool operator==(const Move& other) const {
        return (marbleIndices == other.marbleIndices &&
                direction == other.direction &&
                isInline == other.isInline &&
                pushCount == other.pushCount);
    }
};

//------------------------------------------------------------------------------
// Board Class Declaration
//------------------------------------------------------------------------------

class Board {
public:
    static const int NUM_CELLS = 61;     // Exactly 61 valid positions.
    static const int NUM_DIRECTIONS = 6;   // Directions: W, E, NW, NE, SW, SE.

    // Arrays to track player coordinates
    std::vector<std::pair<int, int>> blackOccupantsCoords;
    std::vector<std::pair<int, int>> whiteOccupantsCoords;

    static std::unordered_map<long long, int> s_coordToIndex;

    // Direction offsets (dx, dy) in board coordinates.
    // Order: W=(-1,0), E=(+1,0), NW=(0,+1), NE=(+1,+1), SW=(-1,-1), SE=(0,-1)
    static const std::array<std::pair<int, int>, NUM_DIRECTIONS> DIRECTION_OFFSETS;

    // Next-to-move color.
    Occupant nextToMove = Occupant::BLACK;

    // Board representation: occupant[i] tells who occupies cell index i.
    std::array<Occupant, NUM_CELLS> occupant;



    // Neighbors table: for each cell i, neighbors[i][d] gives the neighbor's index in
    // direction d (or -1 if none exists).
    std::array<std::array<int, NUM_DIRECTIONS>, NUM_CELLS> neighbors;

    // Reverse mapping: cell index to coordinate (m,y).
    static std::array<std::pair<int, int>, NUM_CELLS> s_indexToCoord;

    //--------------------------------------------------------------------------
    // Public Methods and Constructors
    //--------------------------------------------------------------------------

    // Constructor: initializes coordinate mapping and neighbor table.
    Board();

    // Static function: Converts a board cell's string notation (e.g., "A1", "H5")
    // to its corresponding cell index (0..60). Returns -1 if the notation is invalid.
    static int notationToIndex(const std::string& notation);

    // Converts a cell index to its board notation (e.g., 0 -> "A1").
    static std::string indexToNotation(int idx);

    bool isGroupAligned(const std::vector<int> &group, int &alignedDirection) const;

    // Attempts to apply a move on a temporary copy of the board.
    // Returns true if the move is legal (applied without error), false otherwise.
    bool tryMove(const std::vector<int>& group, int direction, Move& move) const;

    // Generate candidate column groups for the given side.
    std::set<std::vector<int>> generateColumnGroups(Occupant side) const;


    // Generate all legal moves for a given side.
    std::vector<Move> generateMoves(Occupant side) const;

    // Apply a move to *this* board (modifies occupant[]).
    void applyMove(const Move& m);

    bool isMarbleInDanger(int index, Occupant player) const {
        Occupant opponent = (player == Occupant::BLACK) ? Occupant::WHITE : Occupant::BLACK;
        
        // First check if the marble is on the edge
        bool isOnEdge = false;
        for (int dir = 0; dir < NUM_DIRECTIONS; ++dir) {
            if (neighbors[index][dir] == -1) {
                isOnEdge = true;
                break;
            }
        }
        
        // If not on edge, it's not in immediate danger of being pushed off
        if (!isOnEdge) {
            return false;
        }
        
        // Now check if there's a threatening group of opponent marbles
        for (int dir = 0; dir < NUM_DIRECTIONS; ++dir) {
            int prev = neighbors[index][(dir + 3) % 6];  // Opposite direction (pusher's side)
            int next = neighbors[index][dir];  // Direction toward edge
            
            // Skip if there's no opponent marble in the pushing position
            if (prev == -1 || occupant[prev] != opponent)
                continue;
            
            // Count opponent marbles in a line that could push
            int opponentCount = 1;
            int curr = prev;
            while (opponentCount < 3) {
                curr = neighbors[curr][(dir + 3) % 6];
                if (curr == -1 || occupant[curr] != opponent)
                    break;
                opponentCount++;
            }
            
            // Count friendly marbles in the same line (including our target marble)
            int friendlyCount = 1;  // Start with 1 for the marble we're checking
            curr = neighbors[index][dir];
            while (curr != -1 && occupant[curr] == player) {
                friendlyCount++;
                curr = neighbors[curr][dir];
            }
            
            // A marble is in danger if opponent has more marbles than friendly group
            if (opponentCount >= friendlyCount && next == -1) {
                return true;
            }
        }
        
        return false;
    }
    

    bool isPushMove(const Move& move, Occupant player) const {
        if (!move.isInline || move.marbleIndices.empty())
            return false;
    
        int front = getFrontCell(move.marbleIndices, move.direction);
        int neighbor = neighbors[front][move.direction];
    
        if (neighbor == -1)
            return false;
    
        Occupant opp = (player == Occupant::BLACK) ? Occupant::WHITE : Occupant::BLACK;
        int pushableCount = 0;
        int curr = neighbor;
    
        // Count opponent marbles in the push direction
        while (curr != -1 && occupant[curr] == opp && pushableCount < 3) {
            pushableCount++;
            curr = neighbors[curr][move.direction];
        }
    
        // Our group must be stronger than the number of opponent marbles
        if (pushableCount == 0 || move.marbleIndices.size() <= pushableCount)
            return false;
    
        // Condition 1: Are we pushing a marble off the board?
        bool pushesOffBoard = (curr == -1);
    
        // Condition 2: Is the last cell empty (so push succeeds)?
        bool safePush = (curr != -1 && occupant[curr] == Occupant::EMPTY);
    
        // Optional Condition 3: Directional preference (e.g. center control)
        // We'll assume we prefer not to push toward edges unless gaining an advantage
    
        // Evaluate based on push target
        if (pushesOffBoard)
            return true;  // Always take the opportunity to eliminate a marble
    
        if (safePush) {
            if (pushableCount == 1)
                return true;  // Low-risk positional gain
            else {
                // Avoid risky trades unless we outnumber clearly
                return move.marbleIndices.size() >= 3 && pushableCount == 2;
            }
        }
    
        // If we end up pushing into a blocked position or cannot gain from it, avoid it
        return false;
    }    
    

    // Returns the index of the marble in 'group' that is furthest in the given direction.
    // Uses the dot product with the direction offset to decide which marble is the "front."
    int getFrontCell(const std::vector<int>& group, int direction) const;

    // Converts a move into document notation (e.g., "(b, C5, D5) i → NW").
    static std::string moveToNotation(const Move& m, Occupant side);

    // Returns a string representing the board state (e.g., "C5b,D5b,E4b,...").
    std::string toBoardString() const;

    // Hardcoded starting positions.
    void initStandardLayout();
    void initBelgianDaisyLayout();
    void initGermanDaisyLayout();

    // Loads board configuration from an input file.
    // The file should have two lines: the first indicating the next-to-move ('b' or 'w'),
    // the second listing occupied positions with their occupants.
    bool loadFromInputFile(const std::string& filename);

    // Sets the occupant of a cell given its board notation.
    void setOccupant(const std::string& notation, Occupant who, bool updateCoords = false);

    // Helper: Sets the occupant of a cell by its index.
    void setOccupant(int index, Occupant who) {
        if (index >= 0 && index < NUM_CELLS) {
            occupant[index] = who;
        }
    }

    // Returns the occupant at a given cell index.
    Occupant getOccupant(int index) const {
        return (index >= 0 && index < NUM_CELLS) ? occupant[index] : Occupant::EMPTY;
    }

private:
    //--------------------------------------------------------------------------
    // Static Mapping and Neighbor Calculation
    //--------------------------------------------------------------------------

    // Flag to indicate if the coordinate mapping has been initialized.
    static bool s_mappingInitialized;
    // Mapping from a packed coordinate (m,y) to a cell index.


    // Initializes the coordinate mapping.
    static void initMapping();

    // Builds the neighbor table using the coordinate mapping.
    void initNeighbors();

    void updateOccupantCoordinates();

    void updateOccupantCoordinates(int oldIndex, int newIndex, Occupant occupantType);


    //--------------------------------------------------------------------------
    // Group Generation and Alignment Helpers
    //--------------------------------------------------------------------------

    // Recursively collects all connected groups (up to size 3) of marbles of a given side,
    // starting from cell 'current'. The current group is stored in 'group', and valid groups
    // are inserted into 'result'.
    void dfsGroup(int current, Occupant side, std::vector<int>& group,
        std::set<std::vector<int>>& result) const;

    void scanCoordinateSet(const std::vector<std::vector<std::pair<int, int>>>& coordinateSet,
                              Occupant side, std::set<std::vector<int>>& groupSet, int d, bool isHorizontal) const;


    void scanHorizontal(const std::vector<std::vector<std::pair<int, int>>> &coordinateSet, Occupant side, int d,
                        std::set<std::vector<int>> &groups) const;

    void scanNorthEast(const std::vector<std::vector<std::pair<int, int>>> &coordinateSet, Occupant side, int d,
                       std::set<std::vector<int>> &groups) const;

    void scanNorthWest(const std::vector<std::vector<std::pair<int, int>>> &coordinateSet, Occupant side, int d,
                       std::set<std::vector<int>> &groups) const;

    std::set<std::vector<int>> generateGroups(Occupant side) const;


    //--------------------------------------------------------------------------
    // Coordinate Conversion Helpers (Private)
    //--------------------------------------------------------------------------

    // Converts a coordinate pair (m,y) to board notation (e.g., "A1").
    std::string indexToNotation(const std::pair<int, int>& coord) const;

    // Given a coordinate pair, returns the corresponding cell index.
    // Assumes exactly one cell has that coordinate.
    int s_indexToCoordInverse(const std::pair<int, int>& coord) const;
};

#endif // ABALONE_BOARD_H
