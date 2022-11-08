from ragger.firmware.fatstacks.layouts import ChoiceList, ExitHeader, ExitFooter, InfoFooter, \
    LetterOnlyKeyboard, NavigationHeader, Suggestions, TappableCenter
from ragger.firmware.fatstacks.screen import MetaScreen


class Screen(metaclass=MetaScreen):
    layout_center = TappableCenter
    layout_choice_list = ChoiceList
    layout_keyboard = LetterOnlyKeyboard
    layout_suggestions = Suggestions
    layout_exit_button = ExitHeader
    layout_navigation = NavigationHeader
    layout_info = InfoFooter
    layout_quit_info = ExitFooter

    def exit(self):
        did_raise = False
        try:
            self.exit_button.tap()
        except:
            did_raise = True
        if not did_raise:
            raise RuntimeError("The application did not exit at this state")
