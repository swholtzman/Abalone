from PyQt5.QtWidgets import QGraphicsItem
from PyQt5.QtGui import QBrush, QPen, QColor, QRadialGradient, QFont
from PyQt5.QtCore import Qt, QRectF

class TileView(QGraphicsItem):
    def __init__(self, model, center_x, center_y, diameter=55, gameboard=None):
        super().__init__()
        self.model = model
        self.gameboard = gameboard
        self.diameter = diameter

        # Center the tile at (center_x, center_y)
        # half = diameter / 4
        # self.setPos(center_x - half, center_y - half)

        self._bounding_rect = QRectF(0, 0, diameter, diameter)

    def boundingRect(self):
        return self._bounding_rect

    def paint(self, painter, option, widget=None):
        # Create radial gradients for white and black tiles
        white_gradient = QRadialGradient(self.diameter / 2, self.diameter / 2, self.diameter / 2)
        white_gradient.setColorAt(0.0, QColor("#999999"))
        white_gradient.setColorAt(0.575, QColor("#C8C8C8"))
        white_gradient.setColorAt(1.0, QColor("#FFFFFF"))

        black_gradient = QRadialGradient(self.diameter / 2, self.diameter / 2, self.diameter / 2)
        black_gradient.setColorAt(0.0, QColor("#383838"))
        black_gradient.setColorAt(0.795, QColor("#000000"))

        # Decide which brush to use based on occupancy
        if not self.model.is_occupied:
            brush = QBrush(Qt.transparent)
        else:
            if self.model.player_color and self.model.player_color.lower() == "white":
                brush = QBrush(white_gradient)
            else:
                brush = QBrush(black_gradient)

        # Set pen and text colors dynamically based on is_option state:
        if self.model.is_option:
            pen_color = QColor("#CE4800")
            text_color = QColor("#F6C602")
        else:
            pen_color = QColor("#3D3D3D")
            text_color = QColor("#737383")

        # Draw the tile's ellipse with the chosen pen and brush
        pen = QPen(pen_color, 1)
        painter.setPen(pen)
        painter.setBrush(brush)
        painter.drawEllipse(self.boundingRect())

        # Draw the tile's text in the center, nudged if necessary
        painter.setPen(text_color)
        text_rect = self.boundingRect().adjusted(0, 0, 0, 0)
        painter.setFont(QFont("", 18, QFont.Normal))
        painter.drawText(text_rect, Qt.AlignCenter, self.model.tile_id)

    def mousePressEvent(self, event):
        """
        Called by PyQt when the user clicks this tile in the QGraphicsScene.
        We forward it to the GameBoard. SHIFT detection: event.modifiers() & Qt.ShiftModifier
        """
        if self.gameboard:
            shift_pressed = bool(event.modifiers() & Qt.ShiftModifier)
            col_row = self.model.tile_id_coords  # e.g., (col, row)
            self.gameboard.on_tile_clicked(col_row, shift_pressed)
        # Accept the event so it doesn’t propagate further.
        event.accept()

    def refresh(self):
        self.update()