import time
import sys
import random
import platform
from pathlib import Path
from PyQt5 import QtWidgets

if platform.system() == "Darwin":  # macOS
    sys.path.insert(0, "../build")

import abalone_ai
print(abalone_ai.__file__)

class AgentSecretary:
    def __init__(self, game_view, depth=4, time_limit_ms=5000, tt_size_mb=128):
        self.latest_board = None
        self.game_view = game_view
        self.last_move_time = 0.0
        self.agent_total_time = 0.0
        # Pass AI parameters to the C++ backend
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
        
        # Determine current move count based on current player.
        first_line = board_state.splitlines()[0].strip()
        if first_line.lower() == 'b':
            current_move_count = self.game_view.black_scoreboard_model.num_moves_made
        else:
            current_move_count = self.game_view.white_scoreboard_model.num_moves_made
        print(f"[DEBUG] Move count from scoreboard: {current_move_count}")

        # Retrieve total_move_limit from configuration; default to 100 if not set.
        total_move_limit = (
            self.game_view._config.moves_per_team
            if self.game_view._config is not None else 100
        )
        print(f"[DEBUG] Total moves limit: {total_move_limit}")

        move, updated_board = self.ai.find_best_move(current_move_count, total_move_limit)
        
        # Debug message to show the AI call worked.
        print("[DEBUG] AI move computed successfully.")
        print(f"[DEBUG] Move from C++: {move}")
        print(f"[DEBUG] Move Suggested by Agent: {move}")
        print(f"[DEBUG] Updated board: {updated_board}")

        # Store updated board in case other methods need it.
        self.latest_board = updated_board

        move_time = time.time() - start_time
        self.last_move_time = move_time
        self.agent_total_time += move_time

        return move, move_time  # Return only 2 values to avoid unpacking error

    # Uncomment or add additional methods as needed.
    # @staticmethod
    # def generate_random_move():
    #     players = ["Black", "White"]
    #     player = random.choice(players)
    #     num_tiles = random.randint(1, 3)
    #     from_tiles = [f"{chr(65 + random.randint(0, 8))}{random.randint(1, 9)}" for _ in range(num_tiles)]
    #     to_tiles = [f"{chr(65 + random.randint(0, 8))}{random.randint(1, 9)}" for _ in range(num_tiles)]
    #     return f"{player} moves {{{', '.join(from_tiles)}}} to {{{', '.join(to_tiles)}}}"