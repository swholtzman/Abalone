from typing import TYPE_CHECKING

from game_screen.components.button import BaseButton

if TYPE_CHECKING:
    from game_screen.game_view import GameView

class PauseButton(BaseButton):
    def __init__(self, parent: 'GameView' = None):
        super().__init__("../public/resources/images/Pause_SVG.svg", parent)

    def on_click(self):
        """Pause the game clock."""
        game_view: 'GameView' = self.parent()
        if game_view.black_scoreboard_model.is_active:
            game_view.black_scoreboard_view._timer.stop()
        elif game_view.white_scoreboard_model.is_active:
            game_view.white_scoreboard_view._timer.stop()
        game_view.move_history_model.add_move("Pause button clicked")