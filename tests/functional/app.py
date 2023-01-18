from ragger.firmware.stax.layouts import ExitFooter, CenteredFooter, _Layout, LetterOnlyKeyboard, \
    NavigationHeader, Suggestions
from ragger.firmware.stax.use_cases import UseCaseHomeExt
from ragger.firmware.stax.screen import MetaScreen


class CustomChoiceList(_Layout):

    def choose(self, index: int):
        assert 1 <= index <= 6, "Choice index must be in [1, 6]"
        x, y = (200, 430)
        diff = 80
        self.client.finger_touch(x, y + (index - 1)*diff)


class Screen(metaclass=MetaScreen):
    layout_choice_list = CustomChoiceList
    layout_keyboard = LetterOnlyKeyboard
    layout_suggestions = Suggestions
    layout_navigation = NavigationHeader
    layout_quit_info = ExitFooter
    layout_dismiss = CenteredFooter
    use_case_home = UseCaseHomeExt

    def exit(self):
        did_raise = False
        try:
            self.home.quit()
        except:
            did_raise = True
        if not did_raise:
            raise RuntimeError("The application did not exit at this state")
