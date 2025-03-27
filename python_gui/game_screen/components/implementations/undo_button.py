from typing import TYPE_CHECKING

from python_gui.game_screen.components.button import BaseButton

if TYPE_CHECKING:
    from python_gui.game_screen.game_view import GameView

class UndoButton(BaseButton):
    def __init__(self, parent: 'GameView' = None):
        super().__init__("../public/resources/images/Undo_SVG.svg", parent)

    def on_click(self):
        """Undo the last move, adjust time and pieces."""
        game_view: 'GameView' = self.parent()
        if game_view.move_history_model._moves:
            last_move = game_view.move_history_model._moves.pop()
            game_view.move_history_view.refresh()
            game_view.move_history_model._write_to_file()

            # Switch back the current player and scoreboard
            current_player = game_view.game_board.current_player
            if current_player == "Black":
                game_view.game_board.current_player = "White"
                game_view.black_scoreboard_view.set_active(False)
                game_view.white_scoreboard_view.set_active(True)
            else:
                game_view.game_board.current_player = "Black"
                game_view.white_scoreboard_view.set_active(True)
                game_view.black_scoreboard_view.set_active(False)

            # Revert board state (simplified)
            game_view.game_board.set_layout(
                game_view._config.board_layout,
                game_view._config.host_colour,
                "White" if game_view._config.host_colour.lower() == "black" else "Black"
            )
            game_view.move_history_model.add_move("Undo button clicked")