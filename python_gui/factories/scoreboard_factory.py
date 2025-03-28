from PIL.ImageEnhance import Color

from game_screen.scoreboard.scoreboard_model import ScoreboardModel
from game_screen.scoreboard.scoreboard_view import ScoreboardView

class ScoreboardFactory:
    @staticmethod
    def create_scoreboard(player: str,
                          score: int,
                          num_moves_made: int,
                          turn_time: float,
                          is_active: bool,
                          background_color: str,):
        model = ScoreboardModel(
            player=player,
            score=score,
            num_moves_made=num_moves_made,
            turn_time=turn_time,
            is_active=is_active
        )
        view = ScoreboardView(model=model, background_color=background_color)
        return model, view