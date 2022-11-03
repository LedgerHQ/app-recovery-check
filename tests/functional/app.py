from ragger.firmware.fatstacks.layouts import CancelFooter, ChoiceList, InfoHeader, \
    LetterOnlyKeyboard, NavigationHeader, Suggestions, TappableCenter
from ragger.firmware.fatstacks.screen import MetaScreen


class Screen(metaclass=MetaScreen):
    layout_center = TappableCenter
    layout_choice_list = ChoiceList
    layout_keyboard = LetterOnlyKeyboard
    layout_suggestions = Suggestions
    layout_info = InfoHeader
    layout_navigation = NavigationHeader
    layout_footer = CancelFooter

    def exit(self):
        did_raise = False
        try:
            self.footer.tap()
        except:
            did_raise = True
        if not did_raise:
            raise RuntimeError("The application did not exit at this state")
