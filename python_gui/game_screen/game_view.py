from pathlib import Path
from PyQt5 import QtWidgets, QtCore

from python_gui.agent_secretary import AgentSecretary
from python_gui.factories.scoreboard_factory import ScoreboardFactory
from python_gui.game_screen.components.PausePopup import PausePopup
from python_gui.game_screen.components.implementations.pause_button import PauseButton
from python_gui.game_screen.components.implementations.quit_button import QuitButton
from python_gui.game_screen.components.implementations.restart_button import RestartButton
from python_gui.game_screen.components.implementations.undo_button import UndoButton
from python_gui.game_screen.game_board import GameBoard
from python_gui.game_screen.info_panel.info_panel_model import InfoPanelModel
from python_gui.game_screen.info_panel.info_panel_view import InfoPanelView
from python_gui.game_screen.move_history.move_history_model import MoveHistoryModel
from python_gui.game_screen.move_history.move_history_view import MoveHistoryView
from python_gui.victory_screen.win_screen import WinScreen


class GameView(QtWidgets.QWidget):
    def __init__(self, parent=None, main_app_callback=None):
        super().__init__(parent)
        self.main_app_callback = main_app_callback

        # Create GameBoard and board view.
        self.game_board = GameBoard()
        self.board_view = QtWidgets.QGraphicsView(self.game_board.scene, self)
        self.board_view.setFixedSize(700, 700)

        # Create scoreboards.
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

        # Create Move History.
        self.move_history_model = MoveHistoryModel(Path("move_history.json"))
        self.move_history_view = MoveHistoryView(self, self.move_history_model)
        self.game_board.set_move_callback(self.on_move_made)

        # Create AI Information Panel and Agent Secretary.
        self.info_panel_model = InfoPanelModel()
        # AgentSecretary is created externally with the desired AI parameters.
        # Initially, we create it with default parameters.
        self.agent_secretary = AgentSecretary(self)
        self.info_panel_view = InfoPanelView(self, self.info_panel_model)
        self.game_board.set_ai_update_callback(self.update_ai_information)

        # Create left column: Black Scoreboard (top) and Move History (bottom).
        left_column = QtWidgets.QWidget()
        left_layout = QtWidgets.QVBoxLayout(left_column)
        left_layout.addWidget(self.black_scoreboard_view)
        left_layout.addStretch()
        left_layout.addWidget(self.move_history_view)

        # Create right column: White Scoreboard (top) and Agent Information (bottom).
        right_column = QtWidgets.QWidget()
        right_layout = QtWidgets.QVBoxLayout(right_column)
        right_layout.addWidget(self.white_scoreboard_view)
        right_layout.addStretch()
        right_layout.addWidget(self.info_panel_view)

        # Create content layout: horizontal arrangement of columns.
        content_layout = QtWidgets.QHBoxLayout()
        content_layout.addStretch(1)
        content_layout.addWidget(left_column)
        content_layout.addWidget(self.board_view)
        content_layout.addWidget(right_column)
        content_layout.addStretch(1)

        # Main layout: vertical layout for vertical centering.
        main_layout = QtWidgets.QVBoxLayout(self)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        main_layout.addStretch(1)
        main_layout.addLayout(content_layout)
        main_layout.addStretch(1)

        # Create buttons.
        self.pause_button = PauseButton(self)
        self.undo_button = UndoButton(self)
        self.restart_button = RestartButton(self)
        self.quit_button = QuitButton(self)

        # Add pause popup and state
        self.pause_popup = PausePopup(self)
        self.is_paused = False

        # Connect the resume button signal
        self.pause_popup.resume_button.clicked.connect(self.resume_game)

        # Connect button signals.
        self.pause_button.clicked.connect(self.pause_game)
        self.undo_button.clicked.connect(self.undo_move)
        self.restart_button.clicked.connect(self.restart_game)
        self.quit_button.clicked.connect(self.quit_game)

        # Button layout.
        button_layout = QtWidgets.QHBoxLayout()
        button_layout.addStretch(1)
        button_layout.addWidget(self.pause_button)
        button_layout.addSpacing(10)
        button_layout.addWidget(self.undo_button)
        button_layout.addSpacing(10)
        button_layout.addWidget(self.restart_button)
        button_layout.addSpacing(10)
        button_layout.addWidget(self.quit_button)
        button_layout.addStretch(1)
        main_layout.addLayout(button_layout)

        self.win_screen = WinScreen(self)
        self.win_screen.setGeometry(0, 0, self.width(), self.height())
        self.win_screen.hide()

        self._config = None
        self.match_type = "Human vs Human"  # Default match type.

    def pause_game(self):
        if not self.is_paused:
            self.is_paused = True
            self.pause_popup.show()
            self.pause_popup.raise_()  # Bring the popup to the front
            # Stop the active player's clock
            if self.black_scoreboard_model.is_active:
                self.black_scoreboard_view.timer.stop()
            elif self.white_scoreboard_model.is_active:
                self.white_scoreboard_view.timer.stop()
            # Optionally log the pause
            self.move_history_model.add_move("Game paused")

    def resume_game(self):
        if self.is_paused:
            self.is_paused = False
            self.pause_popup.hide()
            # Resume the active player's clock
            if self.black_scoreboard_model.is_active:
                self.black_scoreboard_view.timer.start()
            elif self.white_scoreboard_model.is_active:
                self.white_scoreboard_view.timer.start()
            # Optionally log the resume
            self.move_history_model.add_move("Game resumed")

    def undo_move(self):
        self.undo_button.on_click()

    def restart_game(self):
        self.restart_button.on_click()

    def quit_game(self):
        self.quit_button.on_click()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        if hasattr(self, 'win_screen'):
            self.win_screen.setGeometry(0, 0, self.width(), self.height())
        if hasattr(self, 'pause_popup'):
            self.pause_popup.setGeometry(0, 0, self.width(), self.height())

    def on_win_screen_clicked(self):
        self.reset_game()
        self.main_app_callback()

    def update_ai_information(self, board_state):
        """
        Update AI info and highlight the suggested move.
        Only call the C++ backend if:
          - In Human vs Human, no call is made.
          - In Human vs Computer, call only if the current turn is AI.
          - In Computer vs Computer, always call the backend.
        """
        current_player = board_state.splitlines()[0].strip().lower()  # "b" or "w"
        if self.match_type == "Human vs Human":
            return
        if self.match_type == "Human vs Computer":
            # If the host (human) colour matches current turn, skip AI call.
            if self._config.host_colour.lower()[0] == current_player:
                return

        next_move, move_time = self.agent_secretary.send_state_to_agent(board_state)
        parsed_move = self.game_board.parse_move(next_move)
        self.game_board.highlight_suggested_move(parsed_move)
        self.info_panel_model.next_move = next_move
        self.info_panel_model.move_time = move_time
        self.info_panel_model.agent_total_time = self.agent_secretary.agent_total_time
        self.info_panel_view.refresh()

    def on_move_made(self, move_description):
        """Handle move data from GameBoard."""
        self.move_history_model.add_move(move_description)
        self.move_history_view.refresh()
        
        # Force the board view to update and process events so the new move is visible.
        self.board_view.viewport().update()
        QtWidgets.QApplication.processEvents()
        
        if self.black_scoreboard_model.score >= 6 or self.white_scoreboard_model.score >= 6:
            self.show_win_screen()
            return

        board_state = self.game_board.get_board_state()
        current_player = board_state.splitlines()[0].strip().lower()  # "b" or "w"

        if self.match_type == "Human vs Computer":
            if self._config.host_colour.lower()[0] != current_player:
                QtCore.QTimer.singleShot(500, lambda: self.update_ai_information(self.game_board.get_board_state()))
        elif self.match_type == "Computer vs Computer":
            QtCore.QTimer.singleShot(500, lambda: self.update_ai_information(self.game_board.get_board_state()))

    def show_win_screen(self):
        """Display the winning screen."""
        self.win_screen.show()
        self.win_screen.raise_()

    def set_config(self, config_data):
        """Called when the user finalizes the game configuration."""
        self._config = config_data
        board_layout = config_data.board_layout
        # host_colour = config_data.host_colour
        time_limit_black = config_data.time_limit_black
        time_limit_white = config_data.time_limit_white
        self.match_type = config_data.match_type  # e.g., "Human vs Human", "Human vs Computer", etc.

        # NOTE: The new AI parameters (ai_max_depth and ai_time_limit_ms) are expected to be passed in
        # via the settings screen (or MainApp) and used to reinitialize the pybind wrapper (or AgentSecretary)
        # BEFORE calling set_config. Therefore, no reinitialization logic is included here.

        self.black_scoreboard_model.reset()
        self.white_scoreboard_model.reset()
        self.black_scoreboard_model.turn_time_settings = time_limit_black
        self.white_scoreboard_model.turn_time_settings = time_limit_white
        self.black_scoreboard_model.is_active = True  # Black starts.
        self.white_scoreboard_model.is_active = False
        self.black_scoreboard_view.refresh()
        self.white_scoreboard_view.refresh()

        self.move_history_model.clear()
        self.game_board.current_player = "Black"

        self.game_board.set_layout(board_layout)

        # For AI matches, trigger an initial AI update after a delay.
        initial_board_state = self.game_board.get_board_state()
        if self.match_type in ["Computer vs Computer", "Human vs Computer"]:
            QtCore.QTimer.singleShot(500, lambda: self.update_ai_information(initial_board_state))

    def get_board_state(self):
        """Return the current board state."""
        return self.game_board.get_board_state()

    def reset_game(self):
        """Reset all game state to initial conditions."""
        self.black_scoreboard_model.reset()
        self.white_scoreboard_model.reset()
        self.black_scoreboard_model.is_active = True
        self.white_scoreboard_model.is_active = False
        self.black_scoreboard_view.refresh()
        self.white_scoreboard_view.refresh()

        self.move_history_model.clear()
        self.game_board.clear_board()
        self.win_screen.hide()