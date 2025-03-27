from PyQt5 import QtWidgets, QtGui, QtCore
from PyQt5.QtWidgets import QGraphicsPolygonItem
from PyQt5.QtGui import QPolygonF, QColor, QBrush
from PyQt5.QtCore import QPointF
from PyQt5.QtSvg import QGraphicsSvgItem

from python_gui.factories.tile_factory import TileFactory


class GameBoard:
    BOARD_COORDS = [
        (5, 9), (6, 9), (7, 9), (8, 9), (9, 9),
        (4, 8), (5, 8), (6, 8), (7, 8), (8, 8), (9, 8),
        (3, 7), (4, 7), (5, 7), (6, 7), (7, 7), (8, 7), (9, 7),
        (2, 6), (3, 6), (4, 6), (5, 6), (6, 6), (7, 6), (8, 6), (9, 6),
        (1, 5), (2, 5), (3, 5), (4, 5), (5, 5), (6, 5), (7, 5), (8, 5), (9, 5),
        (1, 4), (2, 4), (3, 4), (4, 4), (5, 4), (6, 4), (7, 4), (8, 4),
        (1, 3), (2, 3), (3, 3), (4, 3), (5, 3), (6, 3), (7, 3),
        (1, 2), (2, 2), (3, 2), (4, 2), (5, 2), (6, 2),
        (1, 1), (2, 1), (3, 1), (4, 1), (5, 1)
    ]

    DIRECTIONS = {
        "E": (1, 0),
        "W": (-1, 0),
        "NE": (1, 1),
        "NW": (0, 1),
        "SE": (0, -1),
        "SW": (-1, -1),
    }

    def __init__(self):
        self.scene = QtWidgets.QGraphicsScene()
        self.scene.setSceneRect(0, 0, 680, 680)

        # d = 186.52
        # hex_polygon = QPolygonF([
        #     QPointF(340 - d, 0),  # top-left
        #     QPointF(340 + d, 0),  # top-right
        #     QPointF(680, 340),  # right
        #     QPointF(340 + d, 680),  # bottom-right
        #     QPointF(340 - d, 680),  # bottom-left
        #     QPointF(0, 340)  # left
        # ])
        # hex_item = QGraphicsPolygonItem(hex_polygon)
        # hex_item.setBrush(QBrush(QColor("#1E1E1E")))
        # hex_item.setPen(QtGui.QPen(QtCore.Qt.NoPen))
        # self.scene.addItem(hex_item)

        svg_item = QGraphicsSvgItem("../public/resources/images/BoardBG.svg")
        # Instead of hardcoding, compute the center based on the actual bounding rect.
        scene_rect = self.scene.sceneRect()
        scene_center = QtCore.QPointF(scene_rect.width() / 2, scene_rect.height() / 2)
        svg_rect = svg_item.boundingRect()
        # Set the SVG's position so its center matches the scene center.
        svg_item.setPos(scene_center - svg_rect.center())
        self.scene.addItem(svg_item)

        # Optionally, add a visual marker at the scene center (for debugging).
        center_marker = QtWidgets.QGraphicsEllipseItem(0, 0, 10, 10)
        center_marker.setBrush(QBrush(QColor("red")))
        # Position so that its center is exactly at the scene center.
        center_marker.setPos(scene_center - QtCore.QPointF(5, 5))
        self.scene.addItem(center_marker)

        # Precompute row groupings for centering each row:
        self.rows = {}
        for (col, row) in self.BOARD_COORDS:
            self.rows.setdefault(row, []).append(col)
        for row in self.rows:
            self.rows[row].sort()
        self.sorted_rows = sorted(self.rows.keys(), reverse=True)

        self._tile_factory = TileFactory(tile_size=55, gameboard=self)
        self._tiles = {}
        self._selected_tiles = []
        self.current_player = "Black"

        # By default, let's create all the tile pairs. Initially unoccupied.
        self._create_board()

    def _create_board(self):
        """
        Use TileFactory to create a tile (model + view) for each coordinate,
        place it in self.scene, and store references for logic.
        """
        for (col, row) in self.BOARD_COORDS:
            # Convert (col,row) to (x,y) in the scene
            x_pos, y_pos = self._board_position_to_scene_position(col, row)

            # The factory can create the tile model+view together or individually
            tile_model, tile_view = self._tile_factory.create_tile(col, row, x_pos, y_pos)

            # Add the tile_view (a QGraphicsItem) to the scene
            self.scene.addItem(tile_view)

            # Keep references for later updates
            self._tiles[(col, row)] = (tile_model, tile_view)

    def _board_position_to_scene_position(self, col, row):
        tile_width = 55
        h_gap = 7.5  # Horizontal gap: space between tiles in the same row.
        v_gap = 2.5  # Vertical gap: space between rows.

        # Calculate spacing separately for horizontal and vertical directions.
        h_spacing = tile_width + h_gap
        v_spacing = tile_width + v_gap

        board_cluster_size = 550

        # Horizontal positioning:
        count = len(self.rows[row])
        row_width = count * tile_width + (count - 1) * h_gap
        row_x_offset = (board_cluster_size - row_width) / 2
        index_in_row = self.rows[row].index(col)
        x = row_x_offset + index_in_row * h_spacing

        # Vertical positioning:
        number_of_rows = len(self.sorted_rows)
        total_height = number_of_rows * tile_width + (number_of_rows - 1) * v_gap
        row_y_offset = (board_cluster_size - total_height) / 2
        row_index = self.sorted_rows.index(row)
        y = row_y_offset + row_index * v_spacing

        overall_offset = (680 - board_cluster_size) / 2  # For a scene of 680x680.
        return x + overall_offset, y + overall_offset

    def set_layout(self, layout_name, host_color, opponent_color):
        """
        Called by GameView once the user picks a layout, e.g. 'Standard' or 'Belgian Daisy'.
        We set occupant states in the tile models accordingly.
        """
        if layout_name.lower() == "standard":
            self._set_standard_layout(host_color, opponent_color)
        elif layout_name.lower() == "belgian daisy":
            self._set_belgian_daisy_layout(host_color, opponent_color)
        elif layout_name.lower() == "german daisy":
            self._set_german_daisy_layout(host_color, opponent_color)
        else:
            # Default or fallback
            self._clear_occupants()

    def _clear_occupants(self):
        """Set all tiles to unoccupied."""
        for (model, view) in self._tiles.values():
            model.is_occupied = False
            model.player_color = None
            # let the view refresh
            view.refresh()

    def _fill_layout(self, coordinate_list, player_colour):
        for cpos in coordinate_list:
            if cpos in self._tiles:
                tile_model, tile_view = self._tiles[cpos]
                tile_model.is_occupied = True
                tile_model.player_color = player_colour
                tile_view.refresh()

#
    def _set_standard_layout(self, host_color, opponent_color):
        """
        For 'Standard' layout, place host marbles in certain
        coordinates, opponent marbles in others.
        The rest remain unoccupied.
        """
        self._clear_occupants()

        opponent_coords = [
            (5, 9), (6, 9), (7, 9), (4, 8), (5, 8), (6, 8), (8, 9),
            (7, 8), (8, 8), (9, 8), (5, 7), (6, 7), (7, 7), (9, 9)
        ]

        host_coords = [
            (1, 2), (2, 2), (3, 2), (4, 2), (5, 2), (6, 2), (1, 1),
            (2, 1), (3, 1), (4, 1), (5, 1), (3, 3), (4, 3), (5, 3)
        ]

        self._fill_layout(opponent_coords, opponent_color)
        self._fill_layout(host_coords, host_color)

    def _set_belgian_daisy_layout(self, host_color, opponent_color):
        """
        Another example layout. Populate occupant states accordingly.
        """
        self._clear_occupants()

        opponent_coords = [
            (5,9),(6,9),(8,9),(9,9),(4,8),(5,8),(6,8),
            (7,8),(8,8),(9,8),(4,7),(5,7),(7,7),(8,7)
        ]

        host_coords = [
            (2,3),(3,3),(5,3),(6,3),(1,2),(2,2),(3,2),
            (4,2),(5,2),(6,2),(1,1),(2,1),(4,1),(5,1)
        ]

        self._fill_layout(opponent_coords, opponent_color)
        self._fill_layout(host_coords, host_color)

    def _set_german_daisy_layout(self, host_color, opponent_color):
        """
        Another example layout. Populate occupant states accordingly.
        """
        self._clear_occupants()

        opponent_coords = [
            (4,8),(5,8),(3,7),(4,7),(5,7),(3,6),(4,6),
            (8,8),(9,8),(7,7),(8,7),(9,7),(7,6),(8,6)
        ]

        host_coords = [
            (2,4),(3,4),(1,3),(2,3),(3,3),(1,2),(2,2),
            (6,4),(7,4),(5,3),(6,3),(7,3),(5,2),(6,2)
        ]

        self._fill_layout(opponent_coords, opponent_color)
        self._fill_layout(host_coords, host_color)

# MOVE LOGIC IN PROGRESS

    def on_tile_clicked(self, colrow, shift_pressed):
        """
        Called by a TileView when the user clicks that tile.
        We handle selection logic (possibly partial code).
        """
        tile_model, tile_view = self._tiles[colrow]
        # 1) Check color ownership
        if tile_model.player_color != self.current_player:
            # e.g. user clicked an opponent or empty tile; in pure abalone you might allow no selection
            # or you might just clear selection. Let's just clear if not SHIFT, or ignore if SHIFT.
            if not shift_pressed:
                self._selected_tiles.clear()
                self._clear_options()
            return

        if not shift_pressed:
            # Clear old selection, select this tile
            self._selected_tiles = [colrow]
        else:
            # SHIFT: attempt to add colrow, but only if we remain <= 3 and collinear, contiguous, same color
            new_selection = self._selected_tiles + [colrow]
            if self._is_valid_selection(new_selection):
                self._selected_tiles = new_selection
            else:
                # If invalid, maybe ignore or beep, etc.
                pass

        # Now that we have a new selection, highlight possible moves
        self._highlight_valid_moves()

    def _is_valid_selection(self, coords_list):
        """
        Check if coords_list:
          1) has up to 3 tiles
          2) all same color
          3) contiguous and collinear in one of the 6 directions
        This is just a skeleton example.
        """
        if len(coords_list) > 3:
            return False

        # All same color? We can skip if we only add from on_tile_clicked with color check.
        # So let's assume yes if we got here.

        # Check collinearity & adjacency. For 2 or 3 tiles, they must form a line in one of the DIRECTIONS
        # This can be done by sorting them in some coordinate order and verifying consistent steps.
        # We'll skip the actual logic here and just assume it's correct for brevity.
        return True

    def _highlight_valid_moves(self):
        """Clear all is_option, then set is_option for any tile that is a valid move destination."""
        self._clear_options()
        if not self._selected_tiles:
            return

        # We'll find possible moves for the selected group:
        possible_moves = self._get_moves_for_selection(self._selected_tiles)
        # Each "move" might be a (direction, final_positions, maybe push info),
        # or it could be a single tile coordinate for "where to click to confirm".
        # We'll keep it simple: highlight the new leading destination tile(s).

        for mv in possible_moves:
            # Suppose mv["destinations"] is a list of (col,row) the selection would occupy.
            # or maybe the final "first tile" if you prefer the user to click that spot to confirm.
            # We'll highlight them:
            for cpos in mv["highlight_coords"]:
                mod, view = self._tiles[cpos]
                mod.is_option = True
                view.refresh()

    def _clear_options(self):
        for (model, view) in self._tiles.values():
            if model.is_option:
                model.is_option = False
                view.refresh()

    def _get_moves_for_selection(self, selected_coords):
        """
        Return a list of possible moves, each describing which new positions
        would be occupied if that move is taken, or which tile(s) should
        be highlighted for user to click.
        """
        moves = []

        # For each direction in DIRECTIONS:
        for dir_key, (dx, dy) in self.DIRECTIONS.items():
            # Check if the selection can move in that direction (inline or sidestep).
            # If inline, see if push is legal. If sidestep, see if spaces are free, etc.
            # We'll show a simplified approach for a single or multi-tile shift:

            if self._can_move_in_direction(selected_coords, dx, dy):
                # Suppose we figure out the final coords of the group if it moves that way.
                final_positions = self._compute_destination(selected_coords, dx, dy)
                # We might highlight the last tile or all final positions as "clickable to confirm."
                moves.append({
                    "direction": dir_key,
                    "highlight_coords": final_positions  # or maybe just [ final_positions[-1] ] etc.
                })

        return moves

    def _can_move_in_direction(self, selected_coords, dx, dy):
        """
        Determine if the selected marbles can legally move/push in (dx,dy).
        This is where the bulk of Abalone logic goes:
          - Are we inline or sidestep?
          - If inline, do we push an opponent chain?
          - Are we pushing off the board?
          - Are we pushing our own marble? That’s illegal, etc.
        """
        # Big chunk of logic omitted. We'll just stub it out:
        return True

    def _compute_destination(self, selected_coords, dx, dy):
        """
        Return the final positions if we move the selection by (dx,dy).
        For inline pushes, we might also shift the opponents or push them off the board.
        But for highlighting, we might just show the new location of the "front" marble, etc.
        """
        # As an example: if selected_coords = [(3,3)], dx=1, dy=0 => final = [(4,3)]
        # For multiple marbles, do them all in sorted order.
        # We'll do a simple one-liner:
        return [(c + dx, r + dy) for (c, r) in selected_coords]

    def confirm_move(self, destination_coord):
        """
        Called when user clicks an option tile (or some 'Confirm Move' button).
        Actually apply the move: move the selected marbles, push if needed, etc.
        Then switch turns, clear selection, etc.
        """
        # Implementation detail: find which direction/destination this coordinate belongs to,
        # then do the occupant updates:
        pass