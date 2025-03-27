from PyQt5 import QtWidgets, QtCore
from python_gui.game_screen.game_board import GameBoard


class GameView(QtWidgets.QWidget):
    def __init__(self, parent=None, main_app_callback=None):
        super().__init__(parent)
        self.main_app_callback = main_app_callback

        # Create the GameBoard controller first
        self.game_board = GameBoard()

        # Create a QGraphicsView to show the board
        self.board_view = QtWidgets.QGraphicsView(self.game_board.scene, self)
        self.board_view.setFixedSize(700, 700)

        # Now set up the layout to center the board_view
        self.layout = QtWidgets.QVBoxLayout()
        self.setLayout(self.layout)

        # 1) Top stretch
        self.layout.addStretch()

        # 2) Center horizontally
        h_layout = QtWidgets.QHBoxLayout()
        h_layout.addStretch()
        h_layout.addWidget(self.board_view)
        h_layout.addStretch()
        self.layout.addLayout(h_layout)

        # 3) Bottom stretch
        self.layout.addStretch()

        # A place to store your config
        self._config = None

    def set_config(self, config_data):
        self._config = config_data  # store it if you like

        # Access fields directly on the dataclass:
        board_layout = config_data.board_layout
        host_colour = config_data.host_colour

        # Decide on the opponent color:
        opponent_color = "White" if host_colour.lower() == "black" else "Black"

        self.game_board.set_layout(board_layout, host_color=host_colour, opponent_color=opponent_color)