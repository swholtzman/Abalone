import os

from PyQt5.QtWidgets import (
    QWidget,
    QLabel,
    QVBoxLayout,
    QGridLayout,
    QGraphicsDropShadowEffect,
)
from PyQt5.QtCore import Qt, QTimer, QPropertyAnimation, QEasingCurve
from PyQt5.QtGui import QFont, QColor, QFontDatabase
from PyQt5.QtSvg import QSvgWidget

class ScoreboardView(QWidget):
    def __init__(self, model, svg_path: str, parent=None):
        super().__init__(parent)
        self.model = model
        self.svg_path = svg_path

        # Fixed size for the whole scoreboard
        self.setFixedSize(350, 250)

        # 1) Main layout for this widget
        main_layout = QGridLayout(self)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        self.setLayout(main_layout)

        # 2) Container widget to hold the SVG, so we can apply drop shadow to this container
        self.svg_container = QWidget(self)
        self.svg_container.setFixedSize(350, 250)
        # Make sure it's transparent so the shadow can be seen around the edges
        self.svg_container.setStyleSheet("background-color: transparent;")
        main_layout.addWidget(self.svg_container, 0, 0, alignment=Qt.AlignTop | Qt.AlignLeft)

        # 3) QSvgWidget that displays the scoreboard background, placed inside svg_container
        self.svg_bg = QSvgWidget(self.svg_path, self.svg_container)
        self.svg_bg.setFixedSize(350, 250)
        self.svg_bg.move(0, 0)

        # 4) Apply drop shadow to the container (this was working for you)
        self.shadow = QGraphicsDropShadowEffect(self.svg_container)
        self.shadow.setBlurRadius(15)
        self.shadow.setOffset(0, 0)
        self.shadow.setColor(QColor(255, 255, 255, int(0.10 * 255)))
        self.svg_container.setGraphicsEffect(self.shadow)

        # 5) Labels container on top of the SVG for your text
        self.labels_container = QWidget(self)
        self.labels_container.setAttribute(Qt.WA_TranslucentBackground, True)
        labels_layout = QVBoxLayout(self.labels_container)
        labels_layout.setAlignment(Qt.AlignCenter)
        labels_layout.setContentsMargins(0, 0, 0, 0)
        labels_layout.setSpacing(6)
        main_layout.addWidget(self.labels_container, 0, 0, alignment=Qt.AlignCenter)

        # --- LOAD YOUR CUSTOM FONT ---
        # Build an absolute path starting from the directory of this file.
        # Since your ScoreboardView is deeper than the SettingsView,
        # we go two levels up (from python_gui/game_screen/scoreboard to Abalone)
        font_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../public/resources/fonts/LeagueSpartan-VariableFont_wght.ttf"))
        print("Font path:", font_path)
        font_id = QFontDatabase.addApplicationFont(font_path)
        if font_id < 0:
            print("Failed to load League Spartan font!")
            self.league_spartan_family = ""
        else:
            self.league_spartan_family = QFontDatabase.applicationFontFamilies(font_id)[0]

        # Create a QFont object for title text, weight=400 (QFont.Normal typically corresponds to 400)
        font_title = QFont(self.league_spartan_family, 30)
        font_title.setWeight(QFont.Normal)
        # Smaller font for other labels
        font_small = QFont(self.league_spartan_family, 18)
        font_small.setWeight(QFont.Normal)

        # 6) Create label widgets
        self.player_label = QLabel(self.labels_container)
        self.player_label.setFont(font_title)
        self.player_label.setStyleSheet("background-color: transparent;")
        self.player_label.setAlignment(Qt.AlignCenter)
        labels_layout.addWidget(self.player_label)

        self.score_label = QLabel(self.labels_container)
        self.score_label.setFont(font_title)
        self.score_label.setStyleSheet("background-color: transparent;")
        self.score_label.setAlignment(Qt.AlignCenter)
        labels_layout.addWidget(self.score_label)

        self.moves_label = QLabel(self.labels_container)
        self.moves_label.setFont(font_small)
        self.moves_label.setStyleSheet("background-color: transparent;")
        self.moves_label.setAlignment(Qt.AlignCenter)
        labels_layout.addWidget(self.moves_label)

        self.time_label = QLabel(self.labels_container)
        self.time_label.setFont(font_small)
        self.time_label.setStyleSheet("background-color: transparent;")
        self.time_label.setAlignment(Qt.AlignCenter)
        labels_layout.addWidget(self.time_label)

        # Timer for counting down turn_time
        self._timer = QTimer(self)
        self._timer.setInterval(1000)
        self._timer.timeout.connect(self._on_timer_tick)

        self.refresh()

    def refresh(self):
        """Updates scoreboard labels and triggers shadow animation based on active state."""
        self.player_label.setText(self.model.player)
        self.score_label.setText(str(self.model.score))
        self.moves_label.setText(f"Moves: {self.model.num_moves_made}")
        self.time_label.setText(f"Time: {int(self.model.turn_time)}")

        if self.model.player.lower() == "white":
            text_color = "#000000"
        else:
            text_color = "#FFFFFF"

        for lbl in [self.player_label, self.score_label, self.moves_label, self.time_label]:
            lbl.setStyleSheet(f"color: {text_color}; background-color: transparent;")

        self.animate_shadow(self.model.is_active)

        if self.model.is_active:
            if not self._timer.isActive():
                self._timer.start()
        else:
            if self._timer.isActive():
                self._timer.stop()

    def animate_shadow(self, active: bool):
        """Animate the drop shadow effect based on active state."""
        if active:
            target_blur = 15
            target_color = QColor(206, 72, 0, int(0.50 * 255))
        else:
            target_blur = 15
            target_color = QColor(255, 255, 255, int(0.10 * 255))

        anim_blur = QPropertyAnimation(self.shadow, b"blurRadius", self)
        anim_blur.setDuration(300)
        anim_blur.setEasingCurve(QEasingCurve.InOutQuad)
        anim_blur.setEndValue(target_blur)
        anim_blur.start()

        anim_color = QPropertyAnimation(self.shadow, b"color", self)
        anim_color.setDuration(300)
        anim_color.setEasingCurve(QEasingCurve.InOutQuad)
        anim_color.setEndValue(target_color)
        anim_color.start()

    def set_active(self, active: bool):
        self.model.is_active = active
        if active:
            self.model.turn_time = self.model.initial_time
        self.refresh()

    def _on_timer_tick(self):
        if self.model.is_active:
            self.model.turn_time -= 1
            if self.model.turn_time <= 0:
                self.model.turn_time = 0
                self._timer.stop()
            self.refresh()