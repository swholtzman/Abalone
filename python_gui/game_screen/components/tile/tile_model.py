
class TileModel:
    def __init__(self, tile_id, col, row, is_occupied=False, player_color=None, is_option=False):
        """
        tile_id is a string label, e.g. "(5,9)" or "E6".
        col, row are numeric coordinates for logic.
        """
        self._tile_id = tile_id
        self._col = col
        self._row = row
        self._is_occupied = is_occupied
        self._player_color = player_color
        self._is_option = is_option

    @property
    def tile_id(self):
        return self._tile_id

    @property
    def tile_id_coords(self):
        return self._col, self._row

    @property
    def is_occupied(self):
        return self._is_occupied

    @is_occupied.setter
    def is_occupied(self, value):
        self._is_occupied = value

    @property
    def player_color(self):
        return self._player_color

    @player_color.setter
    def player_color(self, value):
        self._player_color = value

    @property
    def is_option(self):
        return self._is_option

    @is_option.setter
    def is_option(self, value):
        self._is_option = value