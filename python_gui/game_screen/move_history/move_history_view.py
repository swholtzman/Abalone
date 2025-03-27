from PyQt5 import QtWidgets, QtCore, QtGui

class MoveHistoryView(QtWidgets.QWidget):
    def __init__(self, parent, model):
        super().__init__(parent)
        self._model = model
        self.setFixedSize(400, 300)

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
        shadow.setColor(QtGui.QColor(0, 0, 0, 64))  # rgba(0,0,0,0.25) approximation
        self.frame.setGraphicsEffect(shadow)

        # Layout with padding inside the frame
        layout = QtWidgets.QVBoxLayout(self.frame)
        layout.setContentsMargins(20, 20, 20, 20)  # Padding
        layout.setSpacing(10)  # Spacing between widgets

        # Title label
        title_label = QtWidgets.QLabel("Move History")
        title_label.setAlignment(QtCore.Qt.AlignCenter)
        title_label.setStyleSheet("color: white; font-size: 24pt; font-weight: 400;")
        layout.addWidget(title_label)

        # Text edit to display moves
        self.text_edit = QtWidgets.QTextEdit()
        self.text_edit.setReadOnly(True)
        self.text_edit.setStyleSheet(
            "background-color: transparent; color: white; border: none; font-size: 14pt;"
        )
        layout.addWidget(self.text_edit)

        # Main layout to position the frame within the widget
        main_layout = QtWidgets.QVBoxLayout(self)
        main_layout.addWidget(self.frame)

        self.refresh()

    def refresh(self):
        """Update the display with the most recent moves, newest at top."""
        moves = self._model.get_recent_moves(limit=5)
        formatted_moves = "\n".join(moves[::-1])  # Reverse to show newest at top
        self.text_edit.setText(formatted_moves)