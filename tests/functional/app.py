from ledgered.devices import Device, DeviceType

from ragger.firmware.touch.layouts import CenteredFooter, Element, LetterOnlyKeyboard, \
    NavigationHeader, Suggestions
from ragger.firmware.touch.use_cases import UseCaseHomeExt, UseCaseSettings
from ragger.firmware.touch.screen import MetaScreen


class CustomChoiceList(Element):

    def choose(self, index: int, device: Device):
        assert 1 <= index <= 3, "Choice index must be in [1, 3]"
        if device.type == DeviceType.STAX:
            x, y = (200, 130)
            diff = 100
        elif device.type == DeviceType.FLEX:
            x, y = (240, 140)
            diff = 90
        elif device.type == DeviceType.APEX_P:
            x, y = (240, 90)
            diff = 70
        else:
            assert False, f"Device {device.name} not supported"

        self.client.finger_touch(x, y + (index - 1) * diff)


class TouchScreen(metaclass=MetaScreen):
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
