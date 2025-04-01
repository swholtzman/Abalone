import time
import sys
import random

sys.path.insert(0, "../build")
import abalone_ai

class AgentSecretary:
    def __init__(self, game_view, depth=4, time_limit_ms=5000, tt_size_mb=64):
        self.game_view = game_view
        self.last_move_time = 0.0
        self.agent_total_time = 0.0
        self.ai = abalone_ai.AbaloneAI(depth, time_limit_ms, tt_size_mb)

    def get_board_state(self):
        return self.game_view.get_board_state()

    def get_turn_times(self):
        return {
            "Black": self.game_view.black_scoreboard_model.turn_time_settings,
            "White": self.game_view.white_scoreboard_model.turn_time_settings
        }

    def send_state_to_agent(self, board_state):
        print(f"[DEBUG] Board state string: {board_state}")
        start_time = time.time()
        self.ai.parse_board_state(board_state)
        next_move = self.ai.find_best_move()
        print(f"[DEBUG] Move from C++: {next_move}")
        move_time = time.time() - start_time
        self.last_move_time = move_time
        self.agent_total_time += move_time
        return next_move, move_time