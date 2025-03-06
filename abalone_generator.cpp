#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>

// Represents a position on the hexagonal board
struct Position {
    int row; // 0-8 (A-I)
    int col; // 0-8 (1-9)
    
    Position() : row(-1), col(-1) {}
    Position(int r, int c) : row(r), col(c) {}
    Position(char r, char c) : row(r - 'A'), col(c - '1') {}
    
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
    
    bool operator<(const Position& other) const {
        if (row != other.row) return row < other.row;
        return col < other.col;
    }
    
    // Check if position is on the board
    bool isValid() const {
        // Check if row is valid (A-I)
        if (row < 0 || row > 8) return false;
        
        // Check if column is valid (1-9)
        if (col < 0 || col > 8) return false;
        
        // Check if position is within the hexagonal shape
        int minCol = std::max(0, 4 - row);
        int maxCol = std::min(8, 12 - row);
        return col >= minCol && col <= maxCol;
    }
    
    // Convert position to string representation (e.g., "A1")
    std::string toString() const {
        std::string result;
        result += (char)(row + 'A');
        result += (char)(col + '1');
        return result;
    }
};

// Represents a marble on the board
struct Marble {
    Position pos;
    char color; // 'b' for black, 'w' for white
    
    Marble() : color('0') {}
    Marble(Position p, char c) : pos(p), color(c) {}
    Marble(int r, int c, char color) : pos(r, c), color(color) {}
    
    bool operator<(const Marble& other) const {
        if (color != other.color) return color < other.color;
        return pos < other.pos;
    }
    
    std::string toString() const {
        return pos.toString() + color;
    }
};

// Represents a move
struct Move {
    std::vector<Position> from; // Positions of marbles being moved
    std::vector<Position> to;   // Positions where marbles will be moved to
    
    Move() {}
    Move(std::vector<Position> f, std::vector<Position> t) : from(f), to(t) {}
    
    bool operator<(const Move& other) const {
        if (from.size() != other.from.size()) 
            return from.size() < other.from.size();
        
        for (size_t i = 0; i < from.size(); ++i) {
            if (!(from[i] == other.from[i])) 
                return from[i] < other.from[i];
        }
        
        for (size_t i = 0; i < to.size(); ++i) {
            if (!(to[i] == other.to[i])) 
                return to[i] < other.to[i];
        }
        
        return false;
    }
    
    // Converts the move to a string representation
    std::string toString() const {
        if (from.size() == 1) {
            // Single marble move
            return from[0].toString() + "-" + to[0].toString();
        } else {
            // Group move
            std::string result = from[0].toString();
            for (size_t i = 1; i < from.size(); ++i) {
                result += from[i].toString();
            }
            result += "-";
            result += to[0].toString();
            for (size_t i = 1; i < to.size(); ++i) {
                result += to[i].toString();
            }
            return result;
        }
    }
};

// Represents the game board
class Board {
private:
    // Map of positions to marbles
    std::map<Position, Marble> marbles;
    char currentPlayer; // 'b' or 'w'
    
    // Direction vectors for adjacent positions
    const std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {-1, 1}, {0, 1}, {1, 0}, {1, -1}, {0, -1} // NW, NE, E, SE, SW, W
    };
    
public:
    Board() : currentPlayer('b') {}
    
    // Initialize the board from a string representation
    void init(const std::string& playerColor, const std::string& boardStr) {
        currentPlayer = playerColor[0];
        marbles.clear();
        
        std::istringstream iss(boardStr);
        std::string marbleStr;
        
        while (std::getline(iss, marbleStr, ',')) {
            if (marbleStr.size() >= 3) {
                char row = marbleStr[0];
                char col = marbleStr[1];
                char color = marbleStr[2];
                Position pos(row - 'A', col - '1');
                marbles[pos] = Marble(pos, color);
            }
        }
    }
    
    // Get the marble at a given position (if any)
    // Use 'const' on both the method and the Marble pointer it returns:
    const Marble* getMarbleAt(const Position& pos) const {
        auto it = marbles.find(pos);
        if (it != marbles.end()) {
            return &it->second; // Pointer to const Marble
        }
        return nullptr;
    }
    
    // Check if a position is empty
    bool isEmpty(const Position& pos) const {
        return marbles.find(pos) == marbles.end();
    }
    
    // Get all marbles of a given color
    std::vector<Marble> getMarblesOfColor(char color) const {
        std::vector<Marble> result;
        for (const auto& pair : marbles) {
            if (pair.second.color == color) {
                result.push_back(pair.second);
            }
        }
        return result;
    }
    
    // Get adjacent position in a given direction
    Position getAdjacentPosition(const Position& pos, int direction) const {
        Position newPos(pos.row + directions[direction].first, 
                       pos.col + directions[direction].second);
        return newPos;
    }
    
    // Apply a move to the board and return the new board
    Board applyMove(const Move& move) const {
        Board newBoard = *this;
        
        // Remove marbles from their original positions
        for (const Position& pos : move.from) {
            Marble marble = *getMarbleAt(pos);
            newBoard.marbles.erase(pos);
        }
        
        // Place marbles at their new positions
        for (size_t i = 0; i < move.from.size() && i < move.to.size(); ++i) {
            const Position& fromPos = move.from[i];
            const Position& toPos = move.to[i];
            
            // Only place if the 'to' position is valid (not off the board)
            if (toPos.isValid()) {
                Marble marble = *getMarbleAt(fromPos);
                marble.pos = toPos;
                newBoard.marbles[toPos] = marble;
            }
        }
        
        return newBoard;
    }
    
    // Generate all possible moves for the current player
    std::vector<Move> generateMoves() const {
        std::vector<Move> moves;
        
        // Get all marbles of the current player
        std::vector<Marble> playerMarbles = getMarblesOfColor(currentPlayer);
        
        // Generate single marble moves
        for (const Marble& marble : playerMarbles) {
            generateSingleMarbleMoves(marble, moves);
        }
        
        // Generate inline moves (2 and 3 marbles)
        generateInlineMoves(playerMarbles, moves);
        
        // Generate side-step moves
        generateSideStepMoves(playerMarbles, moves);
        
        return moves;
    }
    
private:
    // Generate single marble moves
    void generateSingleMarbleMoves(const Marble& marble, std::vector<Move>& moves) const {
        for (int dir = 0; dir < 6; ++dir) {
            Position nextPos = getAdjacentPosition(marble.pos, dir);
            
            // Check if the position is valid and empty
            if (nextPos.isValid() && isEmpty(nextPos)) {
                Move move;
                move.from.push_back(marble.pos);
                move.to.push_back(nextPos);
                moves.push_back(move);
            }
        }
    }
    
    // Check if positions form a straight line
    bool isInline(const std::vector<Position>& positions) const {
        if (positions.size() <= 1) return true;
        
        // Determine the direction
        int rowDiff = positions[1].row - positions[0].row;
        int colDiff = positions[1].col - positions[0].col;
        
        // Check if all positions follow the same direction
        for (size_t i = 1; i < positions.size(); ++i) {
            if (positions[i].row - positions[i-1].row != rowDiff || 
                positions[i].col - positions[i-1].col != colDiff) {
                return false;
            }
        }
        
        return true;
    }
    
    // Find direction between two positions
    int findDirection(const Position& from, const Position& to) const {
        for (int dir = 0; dir < 6; ++dir) {
            Position next = getAdjacentPosition(from, dir);
            if (next == to) return dir;
        }
        return -1; // Not adjacent
    }
    
    // Generate inline moves (2 and 3 marbles)
    void generateInlineMoves(const std::vector<Marble>& playerMarbles, std::vector<Move>& moves) const {
        // Find groups of 2 and 3 aligned marbles
        for (size_t i = 0; i < playerMarbles.size(); ++i) {
            for (int dir = 0; dir < 6; ++dir) {
                std::vector<Position> group;
                group.push_back(playerMarbles[i].pos);
                
                // Try to find aligned marbles (up to 3)
                Position pos = playerMarbles[i].pos;
                for (int count = 1; count < 3; ++count) {
                    Position nextPos = getAdjacentPosition(pos, dir);
                    const Marble* nextMarble = getMarbleAt(nextPos);

                    if (nextMarble && nextMarble->color == currentPlayer) {
                        group.push_back(nextPos);
                        pos = nextPos;
                    } else {
                        break;
                    }
                }
                
                // If we found at least 2 aligned marbles
                if (group.size() >= 2) {
                    // Try to move the group in the direction of alignment
                    tryInlineMove(group, dir, moves);
                    
                    // Try to move the group perpendicular to the direction of alignment
                    int perpDir1 = (dir + 2) % 6;
                    int perpDir2 = (dir + 4) % 6;
                    trySideStepMove(group, perpDir1, moves);
                    trySideStepMove(group, perpDir2, moves);
                }
            }
        }
    }
    
    // Try to move a group of marbles in-line
    void tryInlineMove(const std::vector<Position>& group, int direction, std::vector<Move>& moves) const {
        // We need to push in the direction of the last marble
        Position frontPos = getAdjacentPosition(group.back(), direction);
        
        // Check if we can push opponent marbles
        std::vector<Position> opponentPositions;
        Position checkPos = frontPos;
        while (checkPos.isValid()) {
            const Marble* marble = getMarbleAt(checkPos);
            if (marble) {
                if (marble->color != currentPlayer) {
                    opponentPositions.push_back(checkPos);
                } else {
                    // We hit our own marble, can't push
                    return;
                }
            } else {
                // Empty position, we can push
                break;
            }
            checkPos = getAdjacentPosition(checkPos, direction);
        }
        
        // If we have more opponent marbles than our marbles, we can't push
        if (opponentPositions.size() > group.size()) return;
        
        // If we reached the edge with opponents, check if we can push them off
        if (!checkPos.isValid() && !opponentPositions.empty()) {
            // We can only push if we have more marbles than opponents
            if (group.size() <= opponentPositions.size()) return;
        }
        
        // Create the move
        Move move;
        move.from = group;
        
        // Calculate new positions for our marbles
        for (size_t i = 0; i < group.size(); ++i) {
            Position newPos;
            if (i == group.size() - 1) {
                // Last marble moves to the front
                newPos = frontPos;
            } else {
                // Other marbles move to the position of the next marble
                newPos = group[i + 1];
            }
            move.to.push_back(newPos);
        }
        
        // Calculate new positions for opponent marbles
        std::vector<Position> newOpponentPositions;
        for (size_t i = 0; i < opponentPositions.size(); ++i) {
            Position newPos;
            if (i == opponentPositions.size() - 1) {
                // Last opponent marble may be pushed off
                newPos = getAdjacentPosition(opponentPositions[i], direction);
            } else {
                // Other opponent marbles move to the position of the next opponent marble
                newPos = opponentPositions[i + 1];
            }
            if (newPos.isValid()) {
                newOpponentPositions.push_back(newPos);
            }
        }
        
        // Apply the move to check if it's valid
        Board newBoard = *this;
        
        // Remove all marbles involved
        for (const Position& pos : group) {
            newBoard.marbles.erase(pos);
        }
        for (const Position& pos : opponentPositions) {
            newBoard.marbles.erase(pos);
        }
        
        // Place our marbles in new positions
        for (size_t i = 0; i < group.size(); ++i) {
            const Position& fromPos = group[i];
            const Position& toPos = move.to[i];
            if (toPos.isValid()) {
                Marble marble = *getMarbleAt(fromPos);
                marble.pos = toPos;
                newBoard.marbles[toPos] = marble;
            }
        }
        
        // Place opponent marbles in new positions
        for (size_t i = 0; i < opponentPositions.size() && i < newOpponentPositions.size(); ++i) {
            const Position& fromPos = opponentPositions[i];
            const Position& toPos = newOpponentPositions[i];
            if (toPos.isValid()) {
                Marble marble = *getMarbleAt(fromPos);
                marble.pos = toPos;
                newBoard.marbles[toPos] = marble;
            }
        }
        
        // Add the move to the list
        moves.push_back(move);
    }
    
    // Generate side-step moves
    void generateSideStepMoves(const std::vector<Marble>& playerMarbles, std::vector<Move>& moves) const {
        // For each group of 2 or 3 marbles in a line, try to move them sideways
        for (int groupSize = 2; groupSize <= 3; ++groupSize) {
            for (size_t i = 0; i < playerMarbles.size(); ++i) {
                for (int dir = 0; dir < 6; ++dir) {
                    std::vector<Position> group;
                    group.push_back(playerMarbles[i].pos);
                    
                    // Try to find aligned marbles
                    Position pos = playerMarbles[i].pos;
                    for (int count = 1; count < groupSize; ++count) {
                        Position nextPos = getAdjacentPosition(pos, dir);
                        const Marble* nextMarble = getMarbleAt(nextPos);
                        
                        if (nextMarble && nextMarble->color == currentPlayer) {
                            group.push_back(nextPos);
                            pos = nextPos;
                        } else {
                            break;
                        }
                    }
                    
                    // If we found a group of the right size
                    if (group.size() == groupSize) {
                        // Try to move the group sideways
                        int sideDir1 = (dir + 2) % 6;
                        int sideDir2 = (dir + 4) % 6;
                        trySideStepMove(group, sideDir1, moves);
                        trySideStepMove(group, sideDir2, moves);
                    }
                }
            }
        }
    }
    
    // Try to move a group of marbles sideways
    void trySideStepMove(const std::vector<Position>& group, int direction, std::vector<Move>& moves) const {
        Move move;
        move.from = group;
        
        // Check if all adjacent positions in the side direction are valid and empty
        for (const Position& pos : group) {
            Position sidePos = getAdjacentPosition(pos, direction);
            if (!sidePos.isValid() || !isEmpty(sidePos)) {
                return; // Cannot move sideways
            }
            move.to.push_back(sidePos);
        }
        
        // Add the move to the list
        moves.push_back(move);
    }
    
public:
    // Convert the board to a string representation (sorted as required)
    std::string toString() const {
        std::vector<Marble> blackMarbles;
        std::vector<Marble> whiteMarbles;
        
        for (const auto& pair : marbles) {
            if (pair.second.color == 'b') {
                blackMarbles.push_back(pair.second);
            } else if (pair.second.color == 'w') {
                whiteMarbles.push_back(pair.second);
            }
        }
        
        // Sort by row, then by column
        auto sortByRowCol = [](const Marble& a, const Marble& b) {
            if (a.pos.row != b.pos.row) return a.pos.row < b.pos.row;
            return a.pos.col < b.pos.col;
        };
        
        std::sort(blackMarbles.begin(), blackMarbles.end(), sortByRowCol);
        std::sort(whiteMarbles.begin(), whiteMarbles.end(), sortByRowCol);
        
        std::vector<Marble> allMarbles;
        allMarbles.insert(allMarbles.end(), blackMarbles.begin(), blackMarbles.end());
        allMarbles.insert(allMarbles.end(), whiteMarbles.begin(), whiteMarbles.end());
        
        std::string result;
        for (size_t i = 0; i < allMarbles.size(); ++i) {
            if (i > 0) result += ",";
            result += allMarbles[i].toString();
        }
        
        return result;
    }
    
    char getCurrentPlayer() const {
        return currentPlayer;
    }
    
    void setCurrentPlayer(char player) {
        currentPlayer = player;
    }
};

// Main function
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }
    
    std::string inputFile = argv[1];
    std::string outputMoveFile = inputFile;
    std::string outputBoardFile = inputFile;
    
    size_t dotPos = inputFile.find_last_of(".");
    if (dotPos != std::string::npos) {
        outputMoveFile = inputFile.substr(0, dotPos) + ".move";
        outputBoardFile = inputFile.substr(0, dotPos) + ".board";
    } else {
        outputMoveFile = inputFile + ".move";
        outputBoardFile = inputFile + ".board";
    }
    
    // Read input file
    std::ifstream inFile(inputFile);
    if (!inFile) {
        std::cerr << "Error opening input file: " << inputFile << std::endl;
        return 1;
    }
    
    std::string playerColor;
    std::string boardState;
    
    std::getline(inFile, playerColor);
    std::getline(inFile, boardState);
    
    inFile.close();
    
    // Initialize board
    Board board;
    board.init(playerColor, boardState);
    
    // Generate all possible moves
    std::vector<Move> moves = board.generateMoves();
    
    // Generate resulting board configurations
    std::vector<Board> resultingBoards;
    for (const Move& move : moves) {
        Board newBoard = board.applyMove(move);
        resultingBoards.push_back(newBoard);
    }
    
    // Write output files
    std::ofstream moveFile(outputMoveFile);
    std::ofstream boardFile(outputBoardFile);
    
    if (!moveFile || !boardFile) {
        std::cerr << "Error opening output files" << std::endl;
        return 1;
    }
    
    for (size_t i = 0; i < moves.size(); ++i) {
        moveFile << moves[i].toString() << std::endl;
        boardFile << resultingBoards[i].toString() << std::endl;
    }
    
    moveFile.close();
    boardFile.close();
    
    std::cout << "Generated " << moves.size() << " possible moves." << std::endl;
    
    return 0;
}
