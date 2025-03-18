import sys
from PyQt5 import QtWidgets, QtGui, QtCore


class IntroView(QtWidgets.QWidget):
    def __init__(self, show_frame_callback, parent=None):
        super().__init__(parent)
        self.show_frame_callback = show_frame_callback
        # Set the overall background color (covers the entire widget)
        self.setStyleSheet("background-color: #131313;")

        # Main layout fills the widget completely.
        main_layout = QtWidgets.QVBoxLayout(self)
        main_layout.setContentsMargins(0, 0, 0, 0)  # No margins so background spans full frame
        main_layout.setSpacing(0)
        main_layout.addStretch()
        # Use stretch items to center the container vertically

        # Create a container for the image and text
        container = QtWidgets.QWidget(self)
        container.setStyleSheet("background-color: #131313;")
        container_layout = QtWidgets.QVBoxLayout(container)
        container_layout.setContentsMargins(0, 0, 0, 0)
        container_layout.setSpacing(10)
        container_layout.setAlignment(QtCore.Qt.AlignCenter)

        # Attempt to load the image
        try:
            pixmap = QtGui.QPixmap("../public/resources/images/Title.png")
            if pixmap.isNull():
                raise Exception("Image failed to load")
            self.img_label = QtWidgets.QLabel(container)
            self.img_label.setPixmap(pixmap)
            self.img_label.setAlignment(QtCore.Qt.AlignCenter)
            container_layout.addWidget(self.img_label)
        except Exception as e:
            print("Error loading PNG:", e)
            self.img_label = QtWidgets.QLabel("[Title.png Placeholder]", container)
            self.img_label.setStyleSheet("color: #EBEBEB; font: 24pt; background-color: #131313;")
            self.img_label.setAlignment(QtCore.Qt.AlignCenter)
            container_layout.addWidget(self.img_label)

        # Create the continue label below the image with a top margin
        self.continue_label = QtWidgets.QLabel("Click anywhere to continue", container)
        self.continue_label.setStyleSheet(
            "color: #EBEBEB; font: 14pt; background-color: #131313; margin-top: 10px;"
        )
        self.continue_label.setAlignment(QtCore.Qt.AlignCenter)
        container_layout.addWidget(self.continue_label)

        # Add the container to the main layout in the center.
        main_layout.addWidget(container, alignment=QtCore.Qt.AlignCenter)
        main_layout.addStretch()  # Stretch at bottom to fill remaining space

        # Setup animation variables and timer
        self._alpha_direction = 1
        self._current_alpha = 50
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self._animate_opacity)
        self.timer.start(50)

    def mousePressEvent(self, event):
        # Call the provided callback when any part of this widget is clicked.
        self.show_frame_callback("Settings")

    def _animate_opacity(self):
        if self._current_alpha <= 30:
            self._alpha_direction = 1
        elif self._current_alpha >= 100:
            self._alpha_direction = -1
        self._current_alpha += 2 * self._alpha_direction

        alpha_255 = int((self._current_alpha / 100) * 255)
        alpha_255 = max(0, min(255, alpha_255))

        baseR, baseG, baseB = (19, 19, 19)
        delta = 235 - 19
        fraction = alpha_255 / 255
        newR = int(baseR + delta * fraction)
        newG = int(baseG + delta * fraction)
        newB = int(baseB + delta * fraction)
        new_color = "#%02X%02X%02X" % (newR, newG, newB)

        self.continue_label.setStyleSheet(
            f"color: {new_color}; font: 14pt; background-color: #131313; margin-top: 50px;"
        )


# Example usage
if __name__ == "__main__":
    def show_frame(name):
        print("Switching to frame:", name)


    app = QtWidgets.QApplication(sys.argv)
    window = IntroView(show_frame)
    window.resize(800, 600)
    window.show()
    sys.exit(app.exec_())
