from pathlib import Path
from PyQt5 import QtWidgets, QtCore

from agent_secretary import AgentSecretary
from factories.scoreboard_factory import ScoreboardFactory
from game_screen.components.implementations.pause_button import PauseButton
from game_screen.components.implementations.quit_button import QuitButton
from game_screen.components.implementations.restart_button import RestartButton
from game_screen.components.implementations.undo_button import UndoButton
from game_screen.game_board import GameBoard
from game_screen.info_panel.info_panel_model import InfoPanelModel
from game_screen.info_panel.info_panel_view import InfoPanelView
from game_screen.move_history.move_history_model import MoveHistoryModel
from game_screen.move_history.move_history_view import MoveHistoryView

class GameView(QtWidgets.QWidget):
    def __init__(self, parent=None, main_app_callback=None):
        super().__init__(parent)
        self.main_app_callback = main_app_callback

        # Create GameBoard and board view
        self.game_board = GameBoard()
        self.board_view = QtWidgets.QGraphicsView(self.game_board.scene, self)
        self.board_view.setFixedSize(700, 700)

        # Create scoreboards
        black_model, black_sb = ScoreboardFactory.create_scoreboard(
            player="Black", score=0, num_moves_made=0, turn_time=60.0, is_active=True,
            background_color="#000000"
        )
        white_model, white_sb = ScoreboardFactory.create_scoreboard(
            player="White", score=0, num_moves_made=0, turn_time=60.0, is_active=False,
            background_color="#CaC9Cd"
        )
        self.black_scoreboard_model = black_model
        self.black_scoreboard_view = black_sb
        self.white_scoreboard_model = white_model
        self.white_scoreboard_view = white_sb
        self.game_board.black_scoreboard_model = black_model
        self.game_board.black_scoreboard_view = black_sb
        self.game_board.white_scoreboard_model = white_model
        self.game_board.white_scoreboard_view = white_sb

        # Create Move History
        self.move_history_model = MoveHistoryModel(Path("move_history.json"))
        self.move_history_view = MoveHistoryView(self, self.move_history_model)
        self.game_board.set_move_callback(self.on_move_made)

        # Create AI Information Panel
        self.info_panel_model = InfoPanelModel()
        self.agent_secretary = AgentSecretary(self)
        self.info_panel_view = InfoPanelView(self, self.info_panel_model)
        self.game_board.set_ai_update_callback(self.update_ai_information)

        # **Create left column: Black Scoreboard (top) and Move History (bottom)**
        left_column = QtWidgets.QWidget()
        left_layout = QtWidgets.QVBoxLayout(left_column)
        left_layout.addWidget(self.black_scoreboard_view)  # Scoreboard at the top
        left_layout.addStretch()  # Stretch pushes Move History to the bottom
        left_layout.addWidget(self.move_history_view)  # Move History at the bottom

        # **Create right column: White Scoreboard (top) and Agent Information (bottom)**
        right_column = QtWidgets.QWidget()
        right_layout = QtWidgets.QVBoxLayout(right_column)
        right_layout.addWidget(self.white_scoreboard_view)  # Scoreboard at the top
        right_layout.addStretch()  # Stretch pushes Agent Info to the bottom
        right_layout.addWidget(self.info_panel_view)  # Agent Info at the bottom

        # **Create content layout: horizontal arrangement of columns**
        content_layout = QtWidgets.QHBoxLayout()
        content_layout.addStretch(1)  # Left spacer for horizontal centering
        content_layout.addWidget(left_column)
        content_layout.addWidget(self.board_view)  # Gameboard in the middle
        content_layout.addWidget(right_column)
        content_layout.addStretch(1)  # Right spacer for horizontal centering

        # **Main layout: vertical layout for vertical centering**
        main_layout = QtWidgets.QVBoxLayout(self)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        main_layout.addStretch(1)  # Top spacer for vertical centering
        main_layout.addLayout(content_layout)
        main_layout.addStretch(1)  # Bottom spacer for vertical centering

        # Create buttons
        self.pause_button = PauseButton(self)
        self.undo_button = UndoButton(self)
        self.restart_button = RestartButton(self)
        self.quit_button = QuitButton(self)

        # Connect button signals
        self.pause_button.clicked.connect(self.pause_game)
        self.undo_button.clicked.connect(self.undo_move)
        self.restart_button.clicked.connect(self.restart_game)
        self.quit_button.clicked.connect(self.quit_game)

        # Button layout
        button_layout = QtWidgets.QHBoxLayout()
        button_layout.addStretch(1)  # Left spacer for centering
        button_layout.addWidget(self.pause_button)
        button_layout.addSpacing(10)  # 10px gap between buttons
        button_layout.addWidget(self.undo_button)
        button_layout.addSpacing(10)
        button_layout.addWidget(self.restart_button)
        button_layout.addSpacing(10)
        button_layout.addWidget(self.quit_button)
        button_layout.addStretch(1)  # Right spacer for centering

        # Add to main layout below game board
        main_layout.addLayout(button_layout)

        self._config = None

    def pause_game(self):
        self.pause_button.on_click()

    def undo_move(self):
        self.undo_button.on_click()

    def restart_game(self):
        self.restart_button.on_click()

    def quit_game(self):
        self.quit_button.on_click()

    def update_ai_information(self, board_state):
        """Update the AI Information panel with data from AgentSecretary."""
        next_move, move_time = self.agent_secretary.send_state_to_agent(board_state)
        self.info_panel_model.next_move = next_move
        self.info_panel_model.move_time = move_time
        self.info_panel_model.last_move_time = self.agent_secretary.last_move_time
        self.info_panel_model.agent_total_time = self.agent_secretary.agent_total_time
        self.info_panel_view.refresh()

    def on_move_made(self, move_description):
        """Handle move data from GameBoard."""
        self.move_history_model.add_move(move_description)
        self.move_history_view.refresh()

    def set_config(self, config_data):
        """
        Called when the user finalizes the game config (layout type, time limits, etc.)
        """
        self._config = config_data
        board_layout = config_data.board_layout
        host_colour = config_data.host_colour
        time_limit_black = config_data.time_limit_black
        time_limit_white = config_data.time_limit_white

        # Set scoreboard times
        self.black_scoreboard_model.turn_time_settings = time_limit_black
        self.white_scoreboard_model.turn_time_settings = time_limit_white
        self.black_scoreboard_view.refresh()
        self.white_scoreboard_view.refresh()

        # Decide on the opponent color
        opponent_color = "White" if host_colour.lower() == "black" else "Black"
        self.game_board.set_layout(board_layout, host_color=host_colour, opponent_color=opponent_color)

        # Initial AI update
        initial_board_state = self.game_board.get_board_state()
        self.update_ai_information(initial_board_state)

    def get_board_state(self):
        """Retrieve the current board state."""
        return self.game_board.get_board_state()