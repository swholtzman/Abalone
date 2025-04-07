from PyQt5 import QtWidgets, QtGui, QtCore

class SettingsView(QtWidgets.QWidget):
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller

        self.setStyleSheet("background-color: transparent;")
        main_layout = QtWidgets.QVBoxLayout(self)
        main_layout.setContentsMargins(0, 0, 0, 0)  # Remove external margins
        main_layout.setSpacing(20)

        # Add stretch items to center the content vertically.
        main_layout.addStretch()

        self.center_frame = QtWidgets.QFrame(self)
        self.center_frame.setObjectName("centerFrame")
        self.center_frame.setStyleSheet("""
            QFrame#centerFrame {
                background-color: rgba(52,52,52,0.3);
                border-radius: 10px;
                padding: 20px;
            }
        """)
        self.center_frame.setMinimumSize(800, 650)
        # Add a drop shadow effect (approximating the CSS box-shadow)
        shadow = QtWidgets.QGraphicsDropShadowEffect(self.center_frame)
        shadow.setBlurRadius(12)
        shadow.setOffset(0, 8)
        shadow.setColor(QtGui.QColor(0, 0, 0, 64))
        self.center_frame.setGraphicsEffect(shadow)

        main_layout.addWidget(self.center_frame, alignment=QtCore.Qt.AlignCenter)

        # Change the inner_layout initialization to center its content:
        inner_layout = QtWidgets.QVBoxLayout(self.center_frame)
        inner_layout.setContentsMargins(20, 20, 20, 20)
        inner_layout.setSpacing(25)
        inner_layout.setAlignment(QtCore.Qt.AlignCenter)

        # Change the title label alignment to center instead of left:
        self.title_label = QtWidgets.QLabel("Game Setup", self.center_frame)
        self.title_label.setStyleSheet(
            "color: white; font: 28pt; font-weight: 500; background-color: transparent;"
        )
        self.title_label.setAlignment(QtCore.Qt.AlignLeft)
        inner_layout.addWidget(self.title_label)

        # Create dropdown for Board Layout.
        self._board_layout_combo = self._create_labeled_dropdown(
            parent=self.center_frame,
            layout=inner_layout,
            label_text="Board Layout",
            dropdown_values=["Standard", "Belgian Daisy", "German Daisy"],
            default_value="Standard"
        )

        # Create dropdown for Match Type.
        self._match_type = self._create_labeled_dropdown(
            parent=self.center_frame,
            layout=inner_layout,
            label_text="Match Type",
            dropdown_values=["Human vs Human", "Human vs Computer", "Computer vs Computer"],
            default_value="Human vs Human"
        )

        # Create dropdown for Host Colour.
        self._host_colour_combo = self._create_labeled_dropdown(
            parent=self.center_frame,
            layout=inner_layout,
            label_text="Human/Opposing AI Colour",
            dropdown_values=["Black", "White"],
            default_value="Black"
        )

        # Create integer input for Moves Per Team.
        self._moves_per_team_spin = self._create_labeled_intinput(
            parent=self.center_frame,
            layout=inner_layout,
            label_text="Moves Per Team",
            default_value=50,
            max_value=100
        )

        # Create integer input for Time Limit (Black).
        self._time_limit_black_spin = self._create_labeled_intinput(
            parent=self.center_frame,
            layout=inner_layout,
            label_text="Time Limit (Black)",
            default_value=30,
            max_value=500
        )

        # Create integer input for Time Limit (White).
        self._time_limit_white_spin = self._create_labeled_intinput(
            parent=self.center_frame,
            layout=inner_layout,
            label_text="Time Limit (White)",
            default_value=30,
            max_value=500
        )

        # Create integer input for AI Max Depth.
        self._ai_max_depth_spin = self._create_labeled_intinput(
            parent=self.center_frame,
            layout=inner_layout,
            label_text="AI Max Depth",
            default_value=4,
            max_value=20
        )

        # Create integer input for AI Time Limit (ms).
        self._ai_time_limit_spin = self._create_labeled_intinput(
            parent=self.center_frame,
            layout=inner_layout,
            label_text="AI Time Limit (ms)",
            default_value=5000,
            max_value=10000
        )

        # Add stretch between center frame and the start button.
        main_layout.addSpacing(10)

        # Create the Start Game button below the center frame.
        self.start_button = QtWidgets.QPushButton("Start Game", self)
        self.start_button.setStyleSheet("""
            QPushButton {
                border-radius: 5px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #181612, stop:0.2457 #181612, stop:1 #CE4800);
                font-size: 20px;
                padding: 12px;
                margin-top: 10px;
            }
        """)
        button_shadow = QtWidgets.QGraphicsDropShadowEffect(self.start_button)
        button_shadow.setBlurRadius(10)
        button_shadow.setOffset(0, 4)
        button_shadow.setColor(QtGui.QColor(0, 0, 0, 90))
        self.start_button.setGraphicsEffect(button_shadow)
        self.start_button.setFixedWidth(800)
        self.start_button.setFixedHeight(72)

        self.start_button.clicked.connect(self.on_start_game)
        main_layout.addWidget(self.start_button, alignment=QtCore.Qt.AlignHCenter)

        main_layout.addStretch()  # Push content to center vertically.

    @staticmethod
    def _create_labeled_dropdown(parent, layout, label_text, dropdown_values, default_value):
        container = QtWidgets.QWidget(parent)
        container.setFixedWidth(685)
        container_layout = QtWidgets.QVBoxLayout(container)
        container_layout.setContentsMargins(0, 0, 0, 0)
        container_layout.setSpacing(8)

        label = QtWidgets.QLabel(label_text, container)
        label.setStyleSheet("color: white; background: transparent; font-size: 16px;")
        container_layout.addWidget(label, alignment=QtCore.Qt.AlignLeft)

        frame = QtWidgets.QFrame(container)
        frame.setStyleSheet("""
            QFrame {
                background-color: #FFF;
                border-radius: 7.5px;
            }
        """)
        frame_layout = QtWidgets.QHBoxLayout(frame)
        frame_layout.setContentsMargins(8, 5, 8, 5)
        frame.setFixedHeight(40)

        combo = QtWidgets.QComboBox(frame)
        combo.addItems(dropdown_values)
        combo.setCurrentText(default_value)
        combo.setStyleSheet("color: #150700; font-size: 16px;")
        combo.setEditable(True)
        combo.lineEdit().setAlignment(QtCore.Qt.AlignCenter)
        combo.setEditable(False)
        combo.setFixedHeight(40)

        frame_layout.addWidget(combo)
        container_layout.addWidget(frame)
        layout.addWidget(container, alignment=QtCore.Qt.AlignHCenter)

        return combo

    @staticmethod
    def _create_labeled_intinput(parent, layout, label_text, default_value, max_value):
        container = QtWidgets.QWidget(parent)
        container.setFixedWidth(685)
        container_layout = QtWidgets.QVBoxLayout(container)
        container_layout.setContentsMargins(0, 0, 0, 0)
        container_layout.setSpacing(8)

        label = QtWidgets.QLabel(label_text, container)
        label.setStyleSheet("color: white; background: transparent; font-size: 16px;")
        container_layout.addWidget(label, alignment=QtCore.Qt.AlignLeft)

        frame = QtWidgets.QFrame(container)
        frame.setStyleSheet("""
            QFrame {
                background-color: #FFF;
                border-radius: 7.5px;
            }
        """)
        frame_layout = QtWidgets.QHBoxLayout(frame)
        frame_layout.setContentsMargins(8, 5, 8, 5)
        frame.setFixedHeight(40)

        spin = QtWidgets.QSpinBox(frame)
        spin.setRange(0, max_value)
        spin.setValue(default_value)
        spin.setStyleSheet("color: #150700; font-size: 16px; qproperty-alignment: AlignCenter;")
        spin.setFixedHeight(40)

        frame_layout.addWidget(spin)
        container_layout.addWidget(frame)
        layout.addWidget(container, alignment=QtCore.Qt.AlignHCenter)
        return spin

    def on_start_game(self):
        if hasattr(self.controller, "start_game_clicked"):
            self.controller.start_game_clicked()

    def get_board_layout(self):
        return self._board_layout_combo.currentText()

    def get_match_type(self):
        return self._match_type.currentText()

    def get_host_colour(self):
        return self._host_colour_combo.currentText()

    def get_moves_per_team(self):
        return self._moves_per_team_spin.value()

    def get_time_limit_black(self):
        return self._time_limit_black_spin.value()

    def get_time_limit_white(self):
        return self._time_limit_white_spin.value()

    def get_ai_max_depth(self):
        return self._ai_max_depth_spin.value()

    def get_ai_time_limit_ms(self):
        return self._ai_time_limit_spin.value()