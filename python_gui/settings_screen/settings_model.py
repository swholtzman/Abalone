class SettingsModel:
    def __init__(
        self,
        board_layout="Standard",
        host_colour="Black",
        moves_per_team=50,
        time_limit_black=30,
        time_limit_white=30,
        ai_max_depth=4,
        ai_time_limit_ms=5000,
        is_visible=False
    ):
        self._board_layout = board_layout
        self._host_colour = host_colour
        self._moves_per_team = moves_per_team
        self._time_limit_black = time_limit_black
        self._time_limit_white = time_limit_white
        self._ai_max_depth = ai_max_depth
        self._ai_time_limit_ms = ai_time_limit_ms
        self._is_visible = is_visible

    @property
    def board_layout(self):
        return self._board_layout

    @board_layout.setter
    def board_layout(self, value):
        self._board_layout = value

    @property
    def host_colour(self):
        return self._host_colour

    @host_colour.setter
    def host_colour(self, value):
        self._host_colour = value

    @property
    def moves_per_team(self):
        return self._moves_per_team

    @moves_per_team.setter
    def moves_per_team(self, value):
        self._moves_per_team = value

    @property
    def time_limit_black(self):
        return self._time_limit_black

    @time_limit_black.setter
    def time_limit_black(self, value):
        self._time_limit_black = value

    @property
    def time_limit_white(self):
        return self._time_limit_white

    @time_limit_white.setter
    def time_limit_white(self, value):
        self._time_limit_white = value

    @property
    def ai_max_depth(self):
        return self._ai_max_depth

    @ai_max_depth.setter
    def ai_max_depth(self, value):
        self._ai_max_depth = value

    @property
    def ai_time_limit_ms(self):
        return self._ai_time_limit_ms

    @ai_time_limit_ms.setter
    def ai_time_limit_ms(self, value):
        self._ai_time_limit_ms = value

    @property
    def is_visible(self):
        return self._is_visible

    @is_visible.setter
    def is_visible(self, value):
        self._is_visible = value