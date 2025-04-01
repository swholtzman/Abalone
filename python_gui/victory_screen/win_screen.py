from PyQt5.QtCore import Qt
from PyQt5.QtGui import QPixmap
from PyQt5.QtWidgets import QWidget, QLabel

class WinScreen(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

        # Create a background label for the semi-transparent overlay
        self.background = QLabel(self)
        self.background.setStyleSheet("background-color: rgba(0, 0, 0, 128);")

        # Create and configure the image label
        self.image_label = QLabel(self)
        pixmap = QPixmap("../public/resources/images/victory.png")
        scaled_pixmap = pixmap.scaled(600, 202, Qt.KeepAspectRatio, Qt.SmoothTransformation)
        self.image_label.setPixmap(scaled_pixmap)
        self.image_label.setFixedSize(scaled_pixmap.size())

        # Ensure the image label has no background
        self.image_label.setStyleSheet("background-color: transparent;")

        # Initial positioning (updated in resizeEvent)
        self.image_label.move(0, 0)
        self.background.move(0, 0)

    def resizeEvent(self, event):
        super().resizeEvent(event)
        # Resize the background to cover the entire screen
        self.background.setFixedSize(self.size())

        # Center the image label
        label_size = self.image_label.size()
        new_x = (self.width() - label_size.width()) // 2
        new_y = (self.height() - label_size.height()) // 2
        self.image_label.move(new_x, new_y)

    def mousePressEvent(self, event):
        """Handle click to reset game and return to intro screen."""
        self.parent().on_win_screen_clicked()