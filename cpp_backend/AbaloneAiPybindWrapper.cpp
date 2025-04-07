#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <sstream>
#include "AbaloneAI.h"
#include "Board.h"

class AbaloneAIPybind {
private:
    AbaloneAI ai;
    Board board;

public:
    AbaloneAIPybind(int depth = 4, int timeLimitMs = 5000, size_t ttSizeInMB = 64) 
        : ai(depth, timeLimitMs, ttSizeInMB) {}

    void parse_board_state(const std::string& board_state) {
        board = Board();

        std::istringstream ss(board_state);
        std::string line;

        // First line: current player
        std::getline(ss, line);
        board.nextToMove = (line[0] == 'b') ? Occupant::BLACK : Occupant::WHITE;

        // Second line: marble positions
        std::getline(ss, line);
        std::stringstream marble_ss(line);
        std::string token;
        while (std::getline(marble_ss, token, ',')) {
            if (token.size() < 3) continue;

            char col = token[0];
            int row = token[1] - '0';  // assumes 1-digit row
            Occupant color = (token[2] == 'b') ? Occupant::BLACK : Occupant::WHITE;

            std::string notation = std::string(1, col) + std::to_string(row);
            int index = Board::notationToIndex(notation);
            if (index < 0 || index >= Board::NUM_CELLS) continue;

            board.occupant[index] = color;
            auto [x, y] = Board::s_indexToCoord[index];
            if (color == Occupant::BLACK)
                board.blackOccupantsCoords.emplace_back(x, y);
            else
                board.whiteOccupantsCoords.emplace_back(x, y);
        }
    }

    std::tuple<std::string, std::string> find_best_move(int move_count, int total_moves) {
        // Use a fixed max search depth (e.g., 10) or allow it to be configurable
        int maxSearchDepth = 10;
        auto result = ai.findBestMoveIterativeDeepening(board, maxSearchDepth, move_count, total_moves);
        Occupant side = board.nextToMove;
        std::string moveStr = board.moveToNotation(result.first, side);
        board.applyMove(result.first);
        std::string updatedBoard = board.toBoardString();
        return std::make_tuple(moveStr, updatedBoard);
    }

    std::string get_current_board_string() const {
        return board.toBoardString();
    }

private:
    std::string format_move(const Move& move, Occupant side) {
        return board.moveToNotation(move, side);
    }
};

PYBIND11_MODULE(abalone_ai, m) {
    pybind11::class_<AbaloneAIPybind>(m, "AbaloneAI")
        .def(pybind11::init<int, int, size_t>(),
             pybind11::arg("depth") = 4,
             pybind11::arg("time_limit_ms") = 5000,
             pybind11::arg("tt_size_mb") = 64)
        .def("parse_board_state", &AbaloneAIPybind::parse_board_state)
        .def("find_best_move", &AbaloneAIPybind::find_best_move,
             pybind11::arg("move_count"), pybind11::arg("total_moves"))
        .def("get_current_board_string", &AbaloneAIPybind::get_current_board_string);
}