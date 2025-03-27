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
        game_view.game_board.undo_last_move()
        game_view.move_history_model.add_move("Undo button clicked")