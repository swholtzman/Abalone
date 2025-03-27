import time

from PyQt5 import QtWidgets, QtCore
from PyQt5.QtGui import QColor, QBrush
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

        svg_item = QGraphicsSvgItem("../public/resources/images/BoardBG.svg")
        # Instead of hardcoding, compute the center based on the actual bounding rect.
        scene_rect = self.scene.sceneRect()
        scene_center = QtCore.QPointF(scene_rect.width() / 2, scene_rect.height() / 2)
        svg_rect = svg_item.boundingRect()
        # Set the SVGs position so its center matches the scene center.
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

        self._game_states = []

        self._tile_factory = TileFactory(tile_size=55, gameboard=self)
        self._tiles = {}
        self._selected_tiles = []
        self.current_player = "Black"

        self.black_scoreboard_model = None
        self.black_scoreboard_view = None
        self.white_scoreboard_model = None
        self.white_scoreboard_view = None

        self._create_board()

        self.turn_start_time = time.time()  # Track turn start
        self.move_callback = None  # Callback to GameView
        self.ai_update_callback = None

    def _create_board(self):
        """Use TileFactory to create a tile (model+view) for each board coordinate."""
        for (col, row) in self.BOARD_COORDS:
            x_pos, y_pos = self._board_position_to_scene_position(col, row)
            tile_model, tile_view = self._tile_factory.create_tile(col, row, x_pos, y_pos)
            self.scene.addItem(tile_view)
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
        for c_pos in coordinate_list:
            if c_pos in self._tiles:
                tile_model, tile_view = self._tiles[c_pos]
                tile_model.is_occupied = True
                tile_model.player_color = player_colour
                tile_view.refresh()

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

    def on_tile_clicked(self, col_row, shift_pressed):
        """
        Called by a TileView when the user clicks that tile.
        We handle selection logic (possibly partial code).
        """
        tile_model, tile_view = self._tiles[col_row]

        # CASE 1: If user clicks an option tile, that means they are confirming the move
        if tile_model.is_option:
            # We handle the "confirm move" logic
            self.confirm_move(col_row)
            return

        # CASE 2: Otherwise, we do normal selection
        if tile_model.player_color != self.current_player:
            if not shift_pressed:
                self._selected_tiles.clear()
                self._clear_options()
                self._clear_selected()
            return

        # SHIFT vs Non-SHIFT logic for selection:
        if not shift_pressed:
            self._selected_tiles = [col_row]
        else:
            new_selection = self._selected_tiles + [col_row]
            if self._is_valid_selection(new_selection):
                self._selected_tiles = new_selection
            else:
                pass  # ignore or beep

        self._mark_selected_tiles()
        self._highlight_valid_moves()

    @staticmethod
    def _is_valid_selection(coords_list):
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
            for c_pos in mv["highlight_coords"]:
                mod, view = self._tiles[c_pos]
                mod.is_option = True
                view.refresh()

    def _clear_options(self):
        for (model, view) in self._tiles.values():
            if model.is_option:
                model.is_option = False
                view.refresh()

    def _clear_selected(self):
        for (model, view) in self._tiles.values():
            model.is_selected = False
            view.refresh()

    def _get_moves_for_selection(self, selected_coords):
        """
        Return a list of possible moves.
        For single-tile selection: highlight adjacent unoccupied tiles.
        For 2-3 tile selection:
        """
        if not selected_coords:
            return []

            # SINGLE TILE (unchanged):
        if len(selected_coords) == 1:
            return self._single_tile_moves(selected_coords[0])

            # MULTI TILE (2 or 3)
        alignment_dir, sorted_tiles, is_collinear = self._analyze_selection(selected_coords)
        if not is_collinear:
            return []

        # For each of the 6 directions:
        moves = []
        for dir_key, (dx, dy) in self.DIRECTIONS.items():
            # Figure out if we’re inline or side-step relative to alignment_dir
            is_inline = self._is_inline(alignment_dir, (dx, dy))
            if is_inline:
                # If inline, check push logic or empty-forward logic
                if self._can_move_inline(sorted_tiles, dx, dy):
                    # Compute final positions or highlight
                    final_positions = self._compute_destination(sorted_tiles, dx, dy)
                    moves.append({
                        "direction": dir_key,
                        "highlight_coords": final_positions
                    })
            else:
                # If side-step, each marble must move perpendicular
                if self._can_move_sidestep(sorted_tiles, dx, dy):
                    final_positions = self._compute_destination(sorted_tiles, dx, dy)
                    moves.append({
                        "direction": dir_key,
                        "highlight_coords": final_positions
                    })

        return moves

    def _can_move_inline(self, sorted_tiles, dx, dy):
        """
        Return True if the group can move inline in (dx, dy).
        Checks if the forward space is free or pushable.
        """
        front = self._get_front_most(sorted_tiles, dx, dy)
        f_col, f_row = front
        next_coord = (f_col + dx, f_row + dy)
        if next_coord not in self._tiles:
            # Off-board: suicidal moves are not allowed.
            return False
        next_model, _ = self._tiles[next_coord]
        if not next_model.is_occupied:
            return True  # free space

        if next_model.player_color == self.current_player:
            return False  # cannot push own tile

        # Gather opponent chain.
        chain = self._gather_chain(next_coord, dx, dy)
        chain_len = len(chain)
        selected_len = len(sorted_tiles)
        # FIX: access the tile model from the tuple.
        if any(self._tiles[c][0].player_color == self.current_player for c in chain):
            return False
        if chain_len >= selected_len:
            return False
        # Check if the chain can be pushed further.
        last_in_chain = chain[-1]
        last_col, last_row = last_in_chain
        beyond_coord = (last_col + dx, last_row + dy)
        if beyond_coord not in self._tiles:
            # Pushing off is allowed (capture).
            return True
        else:
            beyond_model, _ = self._tiles[beyond_coord]
            return not beyond_model.is_occupied

    def _can_move_sidestep(self, sorted_tiles, dx, dy):
        """
        For side-step, none of the new positions can be occupied or out of bounds.
        There's no pushing in side-step.
        """
        for (col, row) in sorted_tiles:
            nx, ny = col + dx, row + dy
            if (nx, ny) not in self._tiles:
                return False
            nm, _ = self._tiles[(nx, ny)]
            if nm.is_occupied:
                # side-step can't push, so if it’s occupied -> invalid
                return False
        return True

    def _gather_chain(self, start_col_row, dx, dy):
        """
        Return a list of consecutive marbles (col,row) of the same occupant color
        as the marble at 'start_col_row', in the direction opposite of (dx,dy).
        Because if we push from front -> next -> next, we chain them.
        """
        color = self._tiles[start_col_row][0].player_color
        chain = []
        cur_col_row = start_col_row
        while True:
            if cur_col_row not in self._tiles:
                break
            cm, _ = self._tiles[cur_col_row]
            if cm.is_occupied and cm.player_color == color:
                chain.append(cur_col_row)
                # move deeper in the same direction
                cur_col_row = (cur_col_row[0] + dx, cur_col_row[1] + dy)
            else:
                break
        return chain

    @staticmethod
    def _compute_destination(selected_coords, dx, dy):
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
        if not self._selected_tiles:
            return

        current_state = {
            "board_state": {coord: (model.is_occupied, model.player_color) for coord, (model, _) in
                            self._tiles.items()},
            "current_player": self.current_player,
            "turn_time": (self.black_scoreboard_model.turn_time if self.current_player == "Black"
                          else self.white_scoreboard_model.turn_time),
            "black_score": self.black_scoreboard_model.score,
            "white_score": self.white_scoreboard_model.score,
            "black_total_time": self.black_scoreboard_model.total_time_spent,
            "white_total_time": self.white_scoreboard_model.total_time_spent,
            "black_num_moves": self.black_scoreboard_model.num_moves_made,
            "white_num_moves": self.white_scoreboard_model.num_moves_made,
        }
        self._game_states.append(current_state)

        first_sel = self._selected_tiles[0]
        color_of_selection = self._tiles[first_sel][0].player_color
        scoreboard_model = (self.black_scoreboard_model if color_of_selection.lower() == "black"
                            else self.white_scoreboard_model)

        # Calculate time taken for this move
        time_taken = scoreboard_model.initial_time - scoreboard_model.turn_time
        scoreboard_model.total_time_spent += time_taken
        scoreboard_model.num_moves_made += 1

        # Determine move details
        num_selected = len(self._selected_tiles)
        if num_selected == 1:
            src = self._selected_tiles[0]
            from_coords = [src]
            to_coords = [destination_coord]
            captured = []
            self._move_single(src, destination_coord)
        else:
            alignment_dir, sorted_tiles, is_collinear = self._analyze_selection(self._selected_tiles)
            if not is_collinear:
                return
            possible_moves = self._get_moves_for_selection(self._selected_tiles)
            chosen_move = next((mv for mv in possible_moves if destination_coord in mv["highlight_coords"]), None)
            if not chosen_move:
                return
            dx, dy = self.DIRECTIONS[chosen_move["direction"]]
            from_coords = sorted_tiles
            to_coords = self._compute_destination(sorted_tiles, dx, dy)
            if self._is_inline(alignment_dir, (dx, dy)):
                captured = self._apply_inline_move(sorted_tiles, dx, dy, color_of_selection)
            else:
                self._apply_sidestep_move(sorted_tiles, dx, dy)
                captured = []

        # Format move description
        from_str = [self._coord_to_str(*coord) for coord in from_coords]
        to_str = [self._coord_to_str(*coord) for coord in to_coords]
        captured_str = [self._coord_to_str(*coord) for coord in captured]
        player = self.current_player
        opponent = "White" if player == "Black" else "Black"
        move_description = f"{player} moved {{{', '.join(from_str)}}} to {{{', '.join(to_str)}}}"
        if captured:
            move_description += f", Capturing {opponent} from {', '.join(captured_str)}"
        move_description += f";\n\tThe agent took {time_taken:.2f} seconds; Move time total: {scoreboard_model.total_time_spent:.2f} seconds;\n—"

        # Send move description to GameView
        if self.move_callback:
            self.move_callback(move_description)

        # Clear selections
        self._selected_tiles.clear()
        self._clear_options()
        self._clear_selected()

        # Switch turn
        if self.current_player == "Black":
            self.current_player = "White"
            self.black_scoreboard_view.set_active(False)
            self.white_scoreboard_view.set_active(True)
        else:
            self.current_player = "Black"
            self.white_scoreboard_view.set_active(False)
            self.black_scoreboard_view.set_active(True)
        print(f"Turn ended. Now it's {self.current_player}'s turn.")

        if self.ai_update_callback:
            self.ai_update_callback(self.get_board_state())

        # Generate and print board state
        print(self._generate_board_state())

    def undo_last_move(self):
        if not self._game_states:
            return  # No moves to undo

        last_state = self._game_states.pop()

        # Restore board state
        for coord, (is_occupied, player_color) in last_state["board_state"].items():
            model, view = self._tiles[coord]
            model.is_occupied = is_occupied
            model.player_color = player_color
            view.refresh()

        # Restore current player
        self.current_player = last_state["current_player"]

        # Restore turn_time for the current player
        if self.current_player == "Black":
            self.black_scoreboard_model.turn_time = last_state["turn_time"]
        else:
            self.white_scoreboard_model.turn_time = last_state["turn_time"]

        # Restore scores
        self.black_scoreboard_model.score = last_state["black_score"]
        self.white_scoreboard_model.score = last_state["white_score"]

        # Restore total time spent
        self.black_scoreboard_model.total_time_spent = last_state["black_total_time"]
        self.white_scoreboard_model.total_time_spent = last_state["white_total_time"]

        # Restore number of moves
        self.black_scoreboard_model.num_moves_made = last_state["black_num_moves"]
        self.white_scoreboard_model.num_moves_made = last_state["white_num_moves"]

        # Update scoreboard views
        self.black_scoreboard_view.refresh()
        self.white_scoreboard_view.refresh()

        # Set active player
        if self.current_player == "Black":
            self.black_scoreboard_view.set_active(True)
            self.white_scoreboard_view.set_active(False)
        else:
            self.white_scoreboard_view.set_active(True)
            self.black_scoreboard_view.set_active(False)

        # Clear selections
        self._selected_tiles.clear()
        self._clear_options()
        self._clear_selected()

        # Update AI information if callback exists
        if self.ai_update_callback:
            self.ai_update_callback(self.get_board_state())

    def _apply_inline_move(self, sorted_tiles, dx, dy, moving_color):
        """
        Update occupant states for an inline move (which might include pushing).
        """
        # 1) Push opponents if needed
        front = self._get_front_most(sorted_tiles, dx, dy)
        captured = self._push_opponents_if_any(front, dx, dy, moving_color)

        # 2) Compute all new positions
        new_positions = {coord: (coord[0] + dx, coord[1] + dy) for coord in sorted_tiles}

        # 3) Update occupancy
        for src_coord, dest_coord in new_positions.items():
            if dest_coord not in self._tiles:
                # Destination out-of-bounds is usually not legal, but skip if it occurs
                continue
            dst_model, dst_view = self._tiles[dest_coord]
            dst_model.is_occupied = True
            dst_model.player_color = moving_color
            dst_view.refresh()

        # 4) Clear source tiles
        for src_coord in sorted_tiles:
            if src_coord not in new_positions.values():
                src_model, src_view = self._tiles[src_coord]
                src_model.is_occupied = False
                src_model.player_color = None
                src_view.refresh()

        return captured

    def _push_opponents_if_any(self, front_coord, dx, dy, capturing_color):
        """
        If there's an opponent chain in front of 'front_coord', push it if possible.
        Possibly leads to capturing if they go off-board.
        """
        captured = []
        next_col = front_coord[0] + dx
        next_row = front_coord[1] + dy
        if (next_col, next_row) not in self._tiles:
            return captured

        next_model, _ = self._tiles[(next_col, next_row)]
        if not next_model.is_occupied:
            return captured

        chain = self._gather_chain((next_col, next_row), dx, dy)
        if not chain:
            return captured

        # We move from the last in chain outwards
        chain_reversed = chain[::-1]  # push them from back->front
        for col_row in chain_reversed:
            col, row = col_row
            new_col = col + dx
            new_row = row + dy
            if (new_col, new_row) not in self._tiles:
                # Off-board => capture
                # occupant removed:
                captured.append(col_row)
                self._tiles[col_row][0].is_occupied = False
                self._tiles[col_row][0].player_color = None
                self._tiles[col_row][1].refresh()
                self.capture_piece(capturing_color)

            else:
                new_m, new_v = self._tiles[(new_col, new_row)]
                old_m, old_v = self._tiles[col_row]
                new_m.is_occupied = True
                new_m.player_color = old_m.player_color

                old_m.is_occupied = False
                old_m.player_color = None

                new_v.refresh()
                old_v.refresh()

        return captured

    def _apply_sidestep_move(self, sorted_tiles, dx, dy):
        """
        For side-step moves, we update each tile in an order that prevents overwriting.
        Since side-step moves do not push, we simply move each tile if the destination is free.
        We'll update in descending order of dot product.
        """
        # Sort descending (i.e. highest dot product first) to move front-most first.
        st = sorted(sorted_tiles, key=lambda coord: coord[0]*dx + coord[1]*dy, reverse=True)
        color = self._tiles[st[0]][0].player_color
        for (col, row) in st:
            src_model, src_view = self._tiles[(col, row)]
            dest_coord = (col + dx, row + dy)
            if dest_coord not in self._tiles:
                continue
            dst_model, dst_view = self._tiles[dest_coord]
            dst_model.is_occupied = True
            dst_model.player_color = color
            src_model.is_occupied = False
            src_model.player_color = None
            src_view.refresh()
            dst_view.refresh()

    # HELPER FUNCTIONS
    @staticmethod
    def _is_inline(alignment_dir, move_direction):
        """
        Return True if the move_direction is inline with the alignment_dir.
        That is, if the move direction equals the alignment direction or its exact opposite.
        """
        return move_direction == alignment_dir or move_direction == (-alignment_dir[0], -alignment_dir[1])

    @staticmethod
    def _get_front_most(sorted_tiles, dx, dy):
        """
        Given a list of selected tile coordinates (already sorted in some order),
        return the coordinate of the tile that is "front" relative to the move direction (dx, dy).
        We define “front” as the tile with the highest dot product with (dx, dy).
        """
        front = max(sorted_tiles, key=lambda coord: coord[0]*dx + coord[1]*dy)
        return front

    @staticmethod
    def _should_reverse_movement(dx, dy):
        """
        For multi-tile moves, determine if we should update in reverse order.
        For example, when moving east (dx > 0) or moving north (dy > 0), we update the
        front-most tile first to avoid overwriting source data.
        """
        if dx > 0 or dy > 0:
            return True
        return False

    def _single_tile_moves(self, tile_coord):
        """
        (Already implemented.) Returns adjacent unoccupied moves.
        """
        col, row = tile_coord
        highlight_coords = []
        for _, (dx, dy) in self.DIRECTIONS.items():
            nx, ny = col + dx, row + dy
            if (nx, ny) in self._tiles:
                neighbor_model, _ = self._tiles[(nx, ny)]
                if not neighbor_model.is_occupied:
                    highlight_coords.append((nx, ny))
        if highlight_coords:
            return [{"direction": "adjacent", "highlight_coords": highlight_coords}]
        else:
            return []

    @staticmethod
    def _analyze_selection(selected_coords):
        """
        For multi-tile selection (2–3), determine:
          - The alignment direction as a tuple (dx, dy) normalized to one of the allowed directions.
          - The list of selected coordinates sorted by their dot product with that alignment.
          - Whether the selection is collinear.
        """
        if not selected_coords:
            return (0, 0), selected_coords, False
        if len(selected_coords) == 1:
            return (1, 0), selected_coords, True

        sorted_tiles = sorted(selected_coords)
        first = sorted_tiles[0]
        last = sorted_tiles[-1]
        dx = last[0] - first[0]
        dy = last[1] - first[1]

        if dx > 0 and dy == 0:
            alignment_dir = (1, 0)
        elif dx < 0 and dy == 0:
            alignment_dir = (-1, 0)
        elif dx > 0 and dy > 0:
            alignment_dir = (1, 1)
        elif dx == 0 and dy > 0:
            alignment_dir = (0, 1)
        elif dx == 0 and dy < 0:
            alignment_dir = (0, -1)
        elif dx < 0 and dy < 0:
            alignment_dir = (-1, -1)
        else:
            alignment_dir = (1, 0)

        is_collinear = True
        for i in range(1, len(sorted_tiles)):
            diff = (sorted_tiles[i][0] - sorted_tiles[i - 1][0],
                    sorted_tiles[i][1] - sorted_tiles[i - 1][1])
            if diff != alignment_dir:
                is_collinear = False
                break

        sorted_tiles = sorted(sorted_tiles, key=lambda coord: coord[0] * alignment_dir[0] + coord[1] * alignment_dir[1])
        return alignment_dir, sorted_tiles, is_collinear

    def _move_single(self, src, destination_coord):
        """
        Move a single tile from src to destination_coord.
        """
        src_model, src_view = self._tiles[src]
        dst_model, dst_view = self._tiles[destination_coord]
        if dst_model.is_occupied:
            return
        dst_model.is_occupied = True
        dst_model.player_color = src_model.player_color
        src_model.is_occupied = False
        src_model.player_color = None
        src_view.refresh()
        dst_view.refresh()

    def _mark_selected_tiles(self):
        # Suppose self._selected_tiles is the list of coords
        # First clear .is_selected from all tiles
        for (m, v) in self._tiles.values():
            m.is_selected = False
            v.refresh()

        for coord in self._selected_tiles:
            tile_model, tile_view = self._tiles[coord]
            # If you want to highlight them as selected, set is_selected = True
            # for example if they can't move
            tile_model.is_selected = True
            tile_view.refresh()

    def capture_piece(self, capturing_color):
        """Increment the capturing color's scoreboard by 1."""
        if capturing_color.lower() == "black":
            self.black_scoreboard_model.score += 1
            self.black_scoreboard_view.refresh()
        else:
            self.white_scoreboard_model.score += 1
            self.white_scoreboard_view.refresh()

    @staticmethod
    def _coord_to_str(col, row):
        """Convert (col, row) to display format, e.g., (1,5) -> 'A5'."""
        col_letter = chr(ord('A') + col - 1)
        return f"{col_letter}{row}"

    def _generate_board_state_output(self):
        """Generate board state for C++ algorithm after a move."""
        player_char = 'b' if self.current_player.lower() == "black" else 'w'
        occupied_tiles = []
        for (col, row), (model, _) in self._tiles.items():
            if model.is_occupied:
                tile_str = f"{self._coord_to_str(col, row)}{model.player_color[0].lower()}"
                occupied_tiles.append(tile_str)
        return f"{player_char}\n{','.join(occupied_tiles)}"

    def _generate_board_state(self):
        """Generate board state string for C++ algorithm (e.g., 'w\\nA1b,A2b,...')."""
        player_char = 'b' if self.current_player.lower() == "black" else 'w'
        occupied = []
        for (col, row), (model, _) in self._tiles.items():
            if model.is_occupied:
                tile_str = self._coord_to_str(col, row) + model.player_color[0].lower()
                occupied.append(tile_str)
        # occupied.sort(key=lambda x: (int(x[1]), x[0]))  # Sort by row (x[1]) then column (x[0])
        for tile in occupied:
            if not (tile[0].isalpha() and tile[1].isdigit() and tile[2] in 'wb'):
                raise ValueError(f"Invalid tile string format: {tile}")
        occupied.sort(key=lambda x: (int(x[1:-1]), x[0]))  # Sort by row (x[1:-1]) then column (x[0])
        return f"{player_char}\n{','.join(occupied)}"

    def get_board_state(self):
        return self._generate_board_state()

    def set_move_callback(self, callback):
        """Set the callback function to notify GameView of moves."""
        self.move_callback = callback

    def set_ai_update_callback(self, callback):
        self.ai_update_callback = callback