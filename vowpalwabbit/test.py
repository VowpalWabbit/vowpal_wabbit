# from __future__ import annotations

class Thing:
    def __init__(self, name: "OtherThing"):
        self.name = name


class OtherThing:
    def __init__(self, name: Thing):
        self.name = name
