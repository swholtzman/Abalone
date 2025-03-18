from abc import ABC

class Button(ABC):
    def __init__(self):
        self._icon = None

    @property
    def icon(self):
        return self._icon

    @icon.setter
    def icon(self, icon):
        self._icon = icon

    def on_click(self):
        pass

