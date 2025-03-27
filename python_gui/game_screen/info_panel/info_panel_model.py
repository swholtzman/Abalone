class InfoPanelModel:
    def __init__(self):
        self._next_move = ""
        self._last_move = ""
        self._move_time = 0.0
        self._last_move_time = 0.0
        self._agent_total_time = 0.0

    @property
    def next_move(self):
        return self._next_move

    @next_move.setter
    def next_move(self, value):
        # Shift current next_move to last_move before updating
        self._last_move = self._next_move
        self._next_move = value

    @property
    def last_move(self):
        return self._last_move

    @last_move.setter
    def last_move(self, value):
        self._last_move = value

    @property
    def move_time(self):
        return self._move_time

    @move_time.setter
    def move_time(self, value):
        self._move_time = value

    @property
    def last_move_time(self):
        return self._last_move_time

    @last_move_time.setter
    def last_move_time(self, value):
        self._last_move_time = value

    @property
    def agent_total_time(self):
        return self._agent_total_time

    @agent_total_time.setter
    def agent_total_time(self, value):
        self._agent_total_time = value