# NOTE
#
# This is a copy (not a symlink) of `pong/test/paddle.py` and will have to
# be kept in sync manually. This is due to the way multi_project_tools manages
# copying files and directories to caravel_user_project.

class Paddle(object):
    CYCLE = [1, 1, 0, 0]

    def __init__(self, a, b):
        self.a = a
        self.b = b
        self.a_phase = 2
        self.b_phase = 3
        self.a <= self.CYCLE[self.a_phase]
        self.b <= self.CYCLE[self.b_phase]

    def _turn(self, direction: int) -> None:
        self.a_phase = (self.a_phase + direction) % 4
        self.b_phase = (self.b_phase + direction) % 4
        self.a <= self.CYCLE[self.a_phase]
        self.b <= self.CYCLE[self.b_phase]

    def up(self) -> None:
        self._turn(-1)

    def down(self) -> None:
        self._turn(1)
