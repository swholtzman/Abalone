import os
from PyQt5.QtWidgets import (
    QWidget,
    QLabel,
    QVBoxLayout,
    QFrame,
    QGraphicsDropShadowEffect,
)
from PyQt5.QtCore import Qt, QTimer, QPropertyAnimation, QEasingCurve
from PyQt5.QtGui import QFont, QColor, QFontDatabase

class ScoreboardView(QWidget):
    def __init__(self, model, background_color: str, parent=None):
        super().__init__(parent)
        self.model = model
        self.background_color = background_color

        # Fixed size for the whole scoreboard
        self.setFixedSize(400, 300)

        # Main layout for this widget
        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        self.setLayout(main_layout)

        # Create a QFrame for the background with rounded corners
        self.frame = QFrame(self)
        self.frame.setFixedSize(350, 250)
        self.frame.setStyleSheet(f"""
            QFrame {{
                background-color: {self.background_color};
                border-radius: 10px;
            }}
        """)
        main_layout.addWidget(self.frame, alignment=Qt.AlignCenter)

        # Add a layout to the frame for centering
        frame_layout = QVBoxLayout(self.frame)
        frame_layout.setContentsMargins(20, 20, 20, 20)  # Padding inside the frame
        frame_layout.setSpacing(0)

        # Labels container
        labels_container = QWidget()
        labels_container.setAttribute(Qt.WA_TranslucentBackground, True)
        labels_layout = QVBoxLayout(labels_container)
        labels_layout.setAlignment(Qt.AlignCenter)
        labels_layout.setSpacing(6)

        # Load custom font
        font_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../public/resources/fonts/LeagueSpartan-VariableFont_wght.ttf"))
        font_id = QFontDatabase.addApplicationFont(font_path)
        if font_id < 0:
            print("Failed to load League Spartan font!")
            self.league_spartan_family = ""
        else:
            self.league_spartan_family = QFontDatabase.applicationFontFamilies(font_id)[0]

        # Create fonts
        font_title = QFont(self.league_spartan_family, 30)
        font_title.setWeight(QFont.Normal)
        font_small = QFont(self.league_spartan_family, 18)
        font_small.setWeight(QFont.Normal)

        # Create label widgets
        self.player_label = QLabel()
        self.player_label.setFont(font_title)
        self.player_label.setStyleSheet("background-color: transparent;")
        self.player_label.setAlignment(Qt.AlignCenter)
        labels_layout.addWidget(self.player_label)

        self.score_label = QLabel()
        self.score_label.setFont(font_title)
        self.score_label.setStyleSheet("background-color: transparent;")
        self.score_label.setAlignment(Qt.AlignCenter)
        labels_layout.addWidget(self.score_label)

        self.moves_label = QLabel()
        self.moves_label.setFont(font_small)
        self.moves_label.setStyleSheet("background-color: transparent;")
        self.moves_label.setAlignment(Qt.AlignCenter)
        labels_layout.addWidget(self.moves_label)

        self.time_label = QLabel()
        self.time_label.setFont(font_small)
        self.time_label.setStyleSheet("background-color: transparent;")
        self.time_label.setAlignment(Qt.AlignCenter)
        labels_layout.addWidget(self.time_label)

        # Center the labels_container in the frame
        frame_layout.addStretch(1)
        frame_layout.addWidget(labels_container, alignment=Qt.AlignHCenter)
        frame_layout.addStretch(1)

        # Apply drop shadow to the frame
        self.shadow = QGraphicsDropShadowEffect(self.frame)
        self.shadow.setBlurRadius(20)  # Larger blur for inactive state
        self.shadow.setOffset(0, 0)
        self.shadow.setColor(QColor(255, 255, 255, int(0.10 * 255)))  # Default inactive shadow
        self.frame.setGraphicsEffect(self.shadow)

        # Timer for counting down turn_time
        self._timer = QTimer(self)
        self._timer.setInterval(1000)
        self._timer.timeout.connect(self._on_timer_tick)

        self.refresh()

    @property
    def timer(self):
        return self._timer

    def refresh(self):
        """Updates scoreboard labels and triggers shadow animation based on active state."""
        self.player_label.setText(self.model.player)
        self.score_label.setText(str(self.model.score))
        self.moves_label.setText(f"Moves: {self.model.num_moves_made}")
        self.time_label.setText(f"Time: {int(self.model.turn_time)}")

        # Set text color based on the player
        if self.model.player.lower() == "white":
            text_color = "#000000"  # Black text for white background
        else:
            text_color = "#FFFFFF"  # White text for black background

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
            # Orange shadow for active state, with larger blur
            target_color = QColor(206, 72, 0, int(0.50 * 255))
            target_blur = 30  # Larger blur for active state
        else:
            # Subtle white shadow for inactive state
            target_color = QColor(255, 255, 255, int(0.10 * 255))
            target_blur = 20  # Larger blur for inactive state

        # Animate blur radius
        anim_blur = QPropertyAnimation(self.shadow, b"blurRadius", self)
        anim_blur.setDuration(300)
        anim_blur.setEasingCurve(QEasingCurve.InOutQuad)
        anim_blur.setEndValue(target_blur)
        anim_blur.start()

        # Animate shadow color
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