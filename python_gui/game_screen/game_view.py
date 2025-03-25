
from PyQt5 import QtWidgets
from PyQt5.QtCore import Qt

from python_gui.factories.scoreboard_factory import ScoreboardFactory
from python_gui.game_screen.game_board import GameBoard

class GameView(QtWidgets.QWidget):
    def __init__(self, parent=None, main_app_callback=None):
        super().__init__(parent)
        self.main_app_callback = main_app_callback

        # Create the GameBoard controller (contains the QGraphicsScene)
        self.game_board = GameBoard()
        self.board_view = QtWidgets.QGraphicsView(self.game_board.scene, self)
        self.board_view.setFixedSize(700, 700)

        # 1) Create two scoreboard “views” & “models”
        black_model, black_sb = ScoreboardFactory.create_scoreboard(
            player="Black",
            score=0,
            num_moves_made=0,
            turn_time=60.0,
            is_active=True,
            svg_path="../public/resources/images/black_scoreboard.svg"  # or an absolute path
        )

        white_model, white_sb = ScoreboardFactory.create_scoreboard(
            player="White",
            score=0,
            num_moves_made=0,
            turn_time=60.0,
            is_active=False,
            svg_path="../public/resources/images/white_scoreboard.svg"
        )

        # Store them so we can update them from the board:
        self.black_scoreboard_model = black_model
        self.black_scoreboard_view = black_sb
        self.white_scoreboard_model = white_model
        self.white_scoreboard_view = white_sb

        # Wire them into the GameBoard so it can update scores, moves, etc.
        self.game_board.black_scoreboard_model = black_model
        self.game_board.black_scoreboard_view = black_sb
        self.game_board.white_scoreboard_model = white_model
        self.game_board.white_scoreboard_view = white_sb

        # 2) Build the main layout: scoreboard on left, game board in center, scoreboard on right
        outer_layout = QtWidgets.QHBoxLayout()
        outer_layout.setAlignment(Qt.AlignTop)
        self.setLayout(outer_layout)

        # Left scoreboard (black)
        outer_layout.addWidget(self.black_scoreboard_view, alignment=Qt.AlignTop)

        # Board in the middle
        outer_layout.addWidget(self.board_view, alignment=Qt.AlignTop)

        # Right scoreboard (white)
        outer_layout.addWidget(self.white_scoreboard_view, alignment=Qt.AlignTop)

        # Optionally add a stretch on each side or not:
        # outer_layout.insertStretch(0, 1)  # left stretch
        # outer_layout.addStretch(1)       # right stretch

        # A place to store your config
        self._config = None

    def set_config(self, config_data):
        """
        Called when the user finalizes the game config (layout type, time limits, etc.)
        """
        self._config = config_data  # store it if you like

        # Access fields directly on the dataclass:
        board_layout = config_data.board_layout
        host_colour = config_data.host_colour
        time_limit_black = config_data.time_limit_black
        time_limit_white = config_data.time_limit_white

        # Set scoreboard times
        self.black_scoreboard_model.turn_time_settings = time_limit_black
        self.white_scoreboard_model.turn_time_settings = time_limit_white
        self.black_scoreboard_view.refresh()
        self.white_scoreboard_view.refresh()

        # Decide on the opponent color:
        opponent_color = "White" if host_colour.lower() == "black" else "Black"
        self.game_board.set_layout(board_layout, host_color=host_colour, opponent_color=opponent_color)