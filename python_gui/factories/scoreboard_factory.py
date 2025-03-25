# scoreboard_factory.py
from python_gui.game_screen.scoreboard.scoreboard_model import ScoreboardModel
from python_gui.game_screen.scoreboard.scoreboard_view import ScoreboardView

class ScoreboardFactory:
    @staticmethod
    def create_scoreboard(player: str,
                          score: int,
                          num_moves_made: int,
                          turn_time: float,
                          is_active: bool,
                          svg_path: str):
        model = ScoreboardModel(
            player=player,
            score=score,
            num_moves_made=num_moves_made,
            turn_time=turn_time,
            is_active=is_active
        )
        view = ScoreboardView(model=model, svg_path=svg_path)
        return model, view