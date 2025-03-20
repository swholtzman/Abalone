from python_gui.game_screen.components.tile.tile_model import TileModel
from python_gui.game_screen.components.tile.tile_view import TileView

class TileFactory(object):
    def __init__(self, tile_size=55, gameboard=None):
        self.tile_size = tile_size
        self.gameboard = gameboard

    def create_tile(self, col, row, x, y):
        # tile_id = f"({col},{row})"
        tile_model = TileModel(
            tile_id=f"{chr(ord('A') + (col - 1))}{row}",
            col=col,
            row=row,
            is_occupied=False,
            player_color=None,
            is_option=False
        )

        # 1) Create the tile at local (0,0) so it won’t shift itself yet:
        tile_view = TileView(
            tile_model,
            0,
            0,
            self.tile_size,
            gameboard=self.gameboard
        )

        # 2) Manually center it on (x, y) by shifting half the tile size:
        half_size = self.tile_size / 100 - 1.5
        tile_view.setPos(x - half_size, y - half_size)

        return tile_model, tile_view
