

class ScoreboardModel:
    def __init__(
        self,
        player: str,
        score: int = 0,
        num_moves_made: int = 0,
        turn_time: float = 60.0,
        is_active: bool = False
    ):
        self._player = player
        self._score = score
        self._num_moves_made = num_moves_made
        self._initial_time = turn_time  # store the original turn_time
        self._turn_time = turn_time
        self._is_active = is_active
        self._total_time_spent = 0.0

    @property
    def player(self):
        return self._player

    @property
    def score(self):
        return self._score

    @score.setter
    def score(self, value):
        self._score = value

    @property
    def num_moves_made(self):
        return self._num_moves_made

    @num_moves_made.setter
    def num_moves_made(self, value):
        self._num_moves_made = value

    @property
    def turn_time(self):
        return self._turn_time

    @turn_time.setter
    def turn_time(self, value):
        self._turn_time = value

    @property
    def initial_time(self):
        return self._initial_time

    @property
    def turn_time_settings(self):
        """Used to set new turn times from settings."""
        return self._initial_time

    @turn_time_settings.setter
    def turn_time_settings(self, value: float):
        self._initial_time = value
        self._turn_time = value

    @property
    def is_active(self):
        return self._is_active

    @is_active.setter
    def is_active(self, value):
        self._is_active = value

    @property
    def total_time_spent(self):
        return self._total_time_spent

    @total_time_spent.setter
    def total_time_spent(self, value):
        self._total_time_spent = value