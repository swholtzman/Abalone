from PyQt5 import QtWidgets, QtCore

class PausePopup(QtWidgets.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        # Set a semi-transparent black background (50% opacity)
        self.setStyleSheet("background-color: rgba(0, 0, 0, 128);")
        # Match the size of the parent (GameView)
        self.setGeometry(0, 0, parent.width(), parent.height())

        # Create a centered frame for the popup
        self.pause_frame = QtWidgets.QFrame(self)
        self.pause_frame.setStyleSheet("""
            background-color: white;
            border-radius: 10px;
        """)
        self.pause_frame.setFixedSize(300, 150)

        # Layout for the frame contents
        layout = QtWidgets.QVBoxLayout(self.pause_frame)
        layout.setAlignment(QtCore.Qt.AlignCenter)

        # "Paused" label
        paused_label = QtWidgets.QLabel("Paused", self.pause_frame)
        paused_label.setAlignment(QtCore.Qt.AlignCenter)
        paused_label.setStyleSheet("font-size: 24px; color: black;")
        layout.addWidget(paused_label)

        # "Press to Resume" button
        self.resume_button = QtWidgets.QPushButton("Press to Resume", self.pause_frame)
        self.resume_button.setStyleSheet("font-size: 18px; color: black;")
        layout.addWidget(self.resume_button)

        # Center the frame within the popup
        self.pause_frame.move((self.width() - 300) // 2, (self.height() - 150) // 2)

        # Start hidden
        self.hide()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        # Re-center the pause frame whenever the popup is resized
        self.pause_frame.move((self.width() - 300) // 2, (self.height() - 150) // 2)