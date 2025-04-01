from PyQt5 import QtWidgets, QtGui

class InfoPanelView(QtWidgets.QWidget):
    def __init__(self, parent, model):
        super().__init__(parent)
        self.model = model
        self.setFixedSize(400, 300)  # Match the size of MoveHistoryView for symmetry

        # Create the frame with background and rounded corners
        self.frame = QtWidgets.QFrame(self)
        self.frame.setStyleSheet("""
            QFrame {
                background-color: #151515;
                border-radius: 10px;
            }
        """)
        # Add shadow effect
        shadow = QtWidgets.QGraphicsDropShadowEffect(self.frame)
        shadow.setBlurRadius(12)
        shadow.setOffset(0, 8)
        shadow.setColor(QtGui.QColor(0, 0, 0, 64))
        self.frame.setGraphicsEffect(shadow)

        # Layout with padding
        layout = QtWidgets.QVBoxLayout(self.frame)
        layout.setContentsMargins(20, 20, 20, 20)  # Padding inside the frame
        layout.setSpacing(10)  # Spacing between lines

        # Title
        title_label = QtWidgets.QLabel("Agent Information")
        title_label.setStyleSheet("color: white; font-size: 24pt; font-weight: 400;")
        layout.addWidget(title_label)

        # Information fields
        self.next_move_label = QtWidgets.QLabel()
        self.last_move_label = QtWidgets.QLabel()
        self.move_time_label = QtWidgets.QLabel()
        self.last_move_time_label = QtWidgets.QLabel()
        self.agent_total_time_label = QtWidgets.QLabel()

        fields = [
            ("Next Move:", self.next_move_label),
            ("Last Move:", self.last_move_label),
            ("Move Time (s):", self.move_time_label),
            ("Last Move Time (s):", self.last_move_time_label),
            ("Agent Total Time (s):", self.agent_total_time_label),
        ]

        for field_name, value_label in fields:
            hbox = QtWidgets.QHBoxLayout()
            field_label = QtWidgets.QLabel(field_name)
            field_label.setStyleSheet("color: white; font-size: 14pt;")
            value_label.setStyleSheet("color: white; font-size: 14pt;")
            hbox.addWidget(field_label)
            hbox.addWidget(value_label)
            hbox.addStretch()  # Left-align text
            layout.addLayout(hbox)

        # Main layout to position the frame
        main_layout = QtWidgets.QVBoxLayout(self)
        main_layout.addWidget(self.frame)

        self.refresh()

    def refresh(self):
        self.next_move_label.setText(self.model.next_move)
        self.last_move_label.setText(self.model.last_move)
        self.move_time_label.setText(f"{self.model.move_time:.2f}")
        self.last_move_time_label.setText(f"{self.model.last_move_time:.2f}")
        self.agent_total_time_label.setText(f"{self.model.agent_total_time:.2f}")