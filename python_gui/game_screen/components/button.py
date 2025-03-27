from PyQt5 import QtWidgets, QtGui, QtCore

class BaseButton(QtWidgets.QPushButton):
    def __init__(self, icon_path, parent=None):
        super().__init__(parent)
        self.setFixedSize(210, 75)  # Button size
        icon = QtGui.QIcon(icon_path)
        pixmap = icon.pixmap(QtCore.QSize(200, 60))  # Render SVG at desired size
        self.setIcon(QtGui.QIcon(pixmap))
        self.setIconSize(QtCore.QSize(200, 60))  # Match icon size to pixmap
        self.setStyleSheet("background-color: transparent; border: none;")

        # Create and configure the shadow effect
        shadow = QtWidgets.QGraphicsDropShadowEffect(self)
        shadow.setColor(QtGui.QColor(0, 0, 0, 160))  # Semi-transparent black
        shadow.setBlurRadius(15)  # Shadow spread
        shadow.setOffset(0, 5)  # Vertical offset for depth

        # Apply the shadow effect to the button
        self.setGraphicsEffect(shadow)

    def on_click(self):
        """Override in subclasses to define specific actions."""
        pass