import time
import sys
import random

sys.path.insert(0, "../build")

import abalone_ai
print(abalone_ai.__file__)

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

    # function

    @staticmethod
    def generate_random_move():
        players = ["Black", "White"]
        player = random.choice(players)
        num_tiles = random.randint(1, 3)
        from_tiles = [f"{chr(65 + random.randint(0, 8))}{random.randint(1, 9)}" for _ in range(num_tiles)]
        to_tiles = [f"{chr(65 + random.randint(0, 8))}{random.randint(1, 9)}" for _ in range(num_tiles)]
        return f"{player} moves {{{', '.join(from_tiles)}}} to {{{', '.join(to_tiles)}}}"
    

# Test function here if you want to know where the connection between cpp and python is

    if __name__ == "__main__":
        ai = abalone_ai.AbaloneAI()
        board_state = """b
    C5b,D5b,E4b,E5b,E6b,F5b,F6b,F7b,F8b,G6b,H6b,C3w,C4w,D3w,D4w,D6w,E7w,F4w,G5w,G7w,G8w,G9w,H7w,H8w,H9w
    """
        ai.parse_board_state(board_state)
        move = ai.find_best_move()

        print("[TEST] Move from AI:", move)
        print("[TEST] Updated board state:")
        print(ai.get_current_board_string())