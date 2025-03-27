from PyQt5 import QtWidgets, QtGui, QtCore

class BaseButton(QtWidgets.QPushButton):
    def __init__(self, icon_path, parent=None):
        super().__init__(parent)
        self.setFixedSize(165, 45)  # Set to 165px width, 45px height
        self.setIcon(QtGui.QIcon(icon_path))
        self.setIconSize(QtCore.QSize(40, 40))  # Smaller icon size for aesthetics
        self.setStyleSheet("background-color: transparent; border: none;")  # Transparent background

    def on_click(self):
        """Override in subclasses to define specific actions."""
        pass