from typing import TYPE_CHECKING

from python_gui.game_screen.components.button import BaseButton

if TYPE_CHECKING:
    from python_gui.game_screen.game_view import GameView


class QuitButton(BaseButton):
    def __init__(self, parent: 'GameView' = None):
        super().__init__("../public/resources/images/Quit_SVG.svg", parent)

    def on_click(self):
        """Quit to intro screen."""
        game_view: 'GameView' = self.parent()
        game_view.main_app_callback(None)  # Trigger MainApp's on_quit_game
        game_view.move_history_model.add_move("Quit button clicked")