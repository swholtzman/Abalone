from .settings_model import SettingsModel
from .settings_view import SettingsView


class SettingsController:
    def __init__(self, parent, main_app_callback):
        """
        :param parent: The parent Tk (or Frame) where the SettingsView is placed.
        :param main_app_callback: A function in the MainApp that can be called
                                  when the user clicks 'Start Game'.
        """
        self.model = SettingsModel()  # The data model
        self.view = SettingsView(parent, self)
        self._main_app_callback = main_app_callback

    def start_game_clicked(self):
        """
        Called by SettingsView when user clicks "Start Game".
        Here, you can sync data from view to model, then notify the main app.
        """
        self.model.board_layout = self.view.get_board_layout()
        self.model.match_type = self.view.get_match_type()
        self.model.host_colour = self.view.get_host_colour()
        self.model.moves_per_team = self.view.get_moves_per_team()
        self.model.time_limit_black = self.view.get_time_limit_black()
        self.model.time_limit_white = self.view.get_time_limit_white()

        # Notify main application
        if self._main_app_callback:
            self._main_app_callback(self.model)
