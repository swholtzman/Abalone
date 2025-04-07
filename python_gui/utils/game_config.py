from dataclasses import dataclass

@dataclass
class GameConfig:
    board_layout: str
    match_type: str
    host_colour: str
    moves_per_team: int
    time_limit_black: int
    time_limit_white: int
    ai_max_depth: int
    ai_time_limit_ms: int