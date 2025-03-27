
import time
import random

class AgentSecretary:
    def __init__(self, game_view):
        self.game_view = game_view
        self.last_move_time = 0.0
        self.agent_total_time = 0.0

    def get_board_state(self):
        return self.game_view.get_board_state()

    def get_turn_times(self):
        return {
            "Black": self.game_view.black_scoreboard_model.turn_time_settings,
            "White": self.game_view.white_scoreboard_model.turn_time_settings
        }

    def send_state_to_agent(self, board_state):
        print(f"Sending board state to C++ agent: {board_state}")
        start_time = time.time()
        # Simulate delay
        # delay = random.uniform(0, 5)
        # time.sleep(delay)
        # Generate random move
        next_move = self.generate_random_move()
        end_time = time.time()
        move_time = end_time - start_time
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