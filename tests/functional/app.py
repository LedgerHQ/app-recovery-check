from ragger.firmware.nbgl.layouts import CenteredFooter, Element, LetterOnlyKeyboard, \
    NavigationHeader, Suggestions
from ragger.firmware.nbgl.use_cases import UseCaseHomeExt, UseCaseSettings
from ragger.firmware.nbgl.screen import MetaScreen


class CustomChoiceList(Element):

    def choose(self, index: int):
        assert 1 <= index <= 6, "Choice index must be in [1, 6]"
        x, y = (200, 430)
        diff = 80
        self.client.finger_touch(x, y + (index - 1)*diff)


class StaxScreen(metaclass=MetaScreen):
    # choosing the length a the passphrase. 3 choices
    layout_choice_list = CustomChoiceList
    # entering words
    layout_keyboard = LetterOnlyKeyboard
    # word suggestions. 4 choices
    layout_suggestions = Suggestions
    # going back to the previous screen
    layout_navigation = NavigationHeader
    # Dismiss the final, result screen ("your passphrase is correct / not correct) to go back to the welcome screen
    layout_dismiss = CenteredFooter
    # classic welcome screen with tappable center
    use_case_home = UseCaseHomeExt
    # classic settings screen
    use_case_settings = UseCaseSettings
