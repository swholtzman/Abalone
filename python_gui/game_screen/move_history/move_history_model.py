# from pathlib import Path
# import json
#
# class MoveHistoryModel:
#     def __init__(self, log_file: Path):
#         self._log_file = log_file
#         self._moves = []  # List of move dictionaries
#
#     def add_move(self, move_dict: dict):
#         """Add a move to the list and write to file."""
#         self._moves.append(move_dict)
#         self._write_to_file()
#
#     def get_recent_moves(self, limit=5):
#         """Return the most recent moves, up to the specified limit."""
#         return self._moves[-limit:]
#
#     def _write_to_file(self):
#         """Write the entire move list to the JSON file."""
#         try:
#             with open(self._log_file, 'w') as f:
#                 json.dump(self._moves, f, indent=2)
#         except Exception as e:
#             print(f"Error writing to {self._log_file}: {e}")

from pathlib import Path
import json

class MoveHistoryModel:
    def __init__(self, log_file: Path):
        self._log_file = log_file
        self._moves = []  # List to store move description strings

    def add_move(self, move_description: str):
        """Add a new move to the history and update the JSON file."""
        self._moves.append(move_description)
        self._write_to_file()

    def get_recent_moves(self, limit=5):
        """Return the most recent moves, up to the specified limit."""
        return self._moves[-limit:]

    def _write_to_file(self):
        """Write the entire move history to the JSON file."""
        try:
            with open(self._log_file, 'w') as f:
                json.dump(self._moves, f, indent=2)
        except Exception as e:
            print(f"Error writing to {self._log_file}: {e}")