from typing import TYPE_CHECKING

from game_screen.components.button import BaseButton

if TYPE_CHECKING:
    from game_screen.game_view import GameView


class RestartButton(BaseButton):
    def __init__(self, parent: 'GameView' = None):
        super().__init__("../public/resources/images/Restart_SVG.svg", parent)

    def on_click(self):
        """Reset the game to initial state."""
        game_view: 'GameView' = self.parent()
        game_view.move_history_model._moves.clear()
        game_view.move_history_view.refresh()
        game_view.move_history_model._write_to_file()
        game_view.set_config(game_view._config)  # Reset game state
        game_view.move_history_model.add_move("Restart button clicked")