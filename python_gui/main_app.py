import sys
import os
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtGui import QFontDatabase, QFont

from intro_screen.intro_view import IntroView
from settings_screen.settings_controller import SettingsController

class MainApp(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Abalone")
        self.resize(800, 600)
        self.stacked_widget = QtWidgets.QStackedWidget()
        self.stacked_widget.setStyleSheet("background-color: #131313;")
        self.setCentralWidget(self.stacked_widget)
        self.frames = {}

        # Create the Intro screen.
        intro_view = IntroView(show_frame_callback=self.show_screen)
        self.stacked_widget.addWidget(intro_view)
        self.frames["Intro"] = intro_view

        # Create the Settings screen (via controller + view)
        self.settings_controller = SettingsController(
            parent=self,
            main_app_callback=self.on_start_game
        )
        settings_view = self.settings_controller.view
        self.stacked_widget.addWidget(settings_view)
        self.frames["Settings"] = settings_view

        self.show_screen("Intro")

    def show_screen(self, screen_name):
        if screen_name in self.frames:
            self.stacked_widget.setCurrentWidget(self.frames[screen_name])

    def on_start_game(self, settings_model):
        print("User pressed Start Game!")
        print("Board Layout:", settings_model.board_layout)
        print("Your Colour:", settings_model.your_colour)
        print("Moves Per Team:", settings_model.moves_per_team)
        print("Time Limit Black:", settings_model.time_limit_black)
        print("Time Limit White:", settings_model.time_limit_white)

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)

    font_path = os.path.join(os.path.dirname(__file__), "../public/resources/fonts/LeagueSpartan-VariableFont_wght.ttf")
    print("Font path:", font_path)  # Debug: see the resolved path

    if not os.path.exists(font_path):
        print("Font file does not exist at", font_path)

    font_id = QFontDatabase.addApplicationFont(font_path)
    if font_id != -1:
        font_family = QFontDatabase.applicationFontFamilies(font_id)[0]
        app.setFont(QFont(font_family, 10))
    else:
        print("Failed to load League Spartan font.")

    window = MainApp()
    window.show()
    sys.exit(app.exec_())
