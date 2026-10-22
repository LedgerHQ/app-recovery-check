#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "Mocklcx_hmac.h"
#include "Mocklcx_pbkdf2.h"
#include "Mocklcx_sha256.h"
#include "Mockmain_std_app.h"
#include "Mocknbgl_use_case.h"
#include "Mockos_seed.h"
#include "Mockos_utils.h"
#include "constants.h"
#include "fake_crypto.h"
#include "glyphs.h"
#include "mnemonic.h"
#include "ui.h"
#include "unity.h"

// Default Speculos mnemonic
static const char* const SPECULOS_MNEMONIC[] = {
    "glory", "promote",  "mansion", "idle",    "axis",  "finger",
    "extra", "february", "uncover", "one",     "trip",  "resource",
    "lawn",  "turtle",   "enact",   "monster", "seven", "myth",
    "punch", "hobby",    "comfort", "wild",    "raise", "skin"};
#define SPECULOS_MNEMONIC_SIZE \
    (sizeof(SPECULOS_MNEMONIC) / sizeof(SPECULOS_MNEMONIC[0]))

// Last use case started by the app, with the callbacks a user action triggers
typedef enum {
    UC_NONE,
    UC_HOME,
    UC_LENGTH_CHOICE,
    UC_KEYBOARD,
    UC_RESULT
} use_case_t;

static struct {
    use_case_t use_case;
    // home
    const nbgl_genericContents_t* settings;
    const nbgl_homeAction_t* action;
    nbgl_callback_t quit;
    // length choice
    const char* title;
    nbgl_callback_t length_back;
    nbgl_navCallback_t nav;
    nbgl_layoutTouchCallback_t controls;
    // keyboard
    nbgl_keyboardParams_t keyboard;
    nbgl_callback_t keyboard_back;
    // result
    const nbgl_icon_details_t* icon;
    const char* message;
    nbgl_callback_t close;
} screen;

static void fake_home_and_settings(
    const char* appName, const nbgl_icon_details_t* appIcon,
    const char* tagline, const uint8_t initSettingPage,
    const nbgl_genericContents_t* settingContents,
    const nbgl_contentInfoList_t* infosList, const nbgl_homeAction_t* action,
    nbgl_callback_t quitCallback, int cmock_num_calls) {
    (void)appName;
    (void)tagline;
    (void)infosList;
    (void)cmock_num_calls;
    TEST_ASSERT_EQUAL_PTR(&C_stax_recovery_check_64px, appIcon);
    TEST_ASSERT_EQUAL(INIT_HOME_PAGE, initSettingPage);
    screen.use_case = UC_HOME;
    screen.settings = settingContents;
    screen.action = action;
    screen.quit = quitCallback;
}

static void fake_navigable_content(const char* title, uint8_t initPage,
                                   uint8_t nbPages,
                                   nbgl_callback_t quitCallback,
                                   nbgl_navCallback_t navCallback,
                                   nbgl_layoutTouchCallback_t controlsCallback,
                                   int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_EQUAL(0, initPage);
    TEST_ASSERT_EQUAL(1, nbPages);
    screen.use_case = UC_LENGTH_CHOICE;
    screen.title = title;
    screen.length_back = quitCallback;
    screen.nav = navCallback;
    screen.controls = controlsCallback;
}

static void fake_keyboard(const nbgl_keyboardParams_t* params,
                          nbgl_callback_t backCallback, int cmock_num_calls) {
    (void)cmock_num_calls;
    screen.use_case = UC_KEYBOARD;
    screen.keyboard = *params;
    screen.keyboard_back = backCallback;
}

static void fake_action(const nbgl_icon_details_t* icon, const char* message,
                        const char* actionText, nbgl_callback_t callback,
                        int cmock_num_calls) {
    (void)cmock_num_calls;
    TEST_ASSERT_EQUAL_STRING("Close", actionText);
    screen.use_case = UC_RESULT;
    screen.icon = icon;
    screen.message = message;
    screen.close = callback;
}

static const nbgl_contentSwitch_t* passphrase_switch(void) {
    TEST_ASSERT_EQUAL(UC_HOME, screen.use_case);
    TEST_ASSERT_EQUAL(1, screen.settings->nbContents);
    const nbgl_content_t* content = &screen.settings->contentsList[0];
    TEST_ASSERT_EQUAL(SWITCHES_LIST, content->type);
    TEST_ASSERT_EQUAL(1, content->content.switchesList.nbSwitches);
    return content->content.switchesList.switches;
}

static void toggle_passphrase(void) {
    const nbgl_content_t* content = &screen.settings->contentsList[0];
    content->contentActionCallback(passphrase_switch()->token, 0, 0);
}

static void choose_length(size_t nb_words) {
    char text[16];
    nbgl_pageContent_t content = {0};

    TEST_ASSERT_EQUAL(UC_HOME, screen.use_case);
    screen.action->callback();
    TEST_ASSERT_EQUAL(UC_LENGTH_CHOICE, screen.use_case);
    TEST_ASSERT_TRUE(screen.nav(0, &content));
    TEST_ASSERT_EQUAL(BARS_LIST, content.type);
    snprintf(text, sizeof(text), "%u words", (unsigned int)nb_words);
    for (uint8_t i = 0; i < content.barsList.nbBars; i++) {
        if (strcmp(content.barsList.barTexts[i], text) == 0) {
            screen.controls(content.barsList.tokens[i], i);
            return;
        }
    }
    TEST_FAIL_MESSAGE("no bar for this length");
}

// Types the word, as the device does key after key, and selects its suggestion
static void type_word(const char* word) {
    nbgl_layoutKeyboardContent_t content = {0};
    uint32_t mask = 0;

    TEST_ASSERT_EQUAL(UC_KEYBOARD, screen.use_case);
    TEST_ASSERT_EQUAL(KEYBOARD_WITH_SUGGESTIONS, screen.keyboard.type);
    TEST_ASSERT_EQUAL_STRING("", screen.keyboard.entryBuffer);
    for (size_t len = 1; len <= strlen(word); len++) {
        memcpy(screen.keyboard.entryBuffer, word, len);
        screen.keyboard.entryBuffer[len] = '\0';
        screen.keyboard.suggestionParams.updateButtonsCallback(&content, &mask);
    }
    // a whole word is the first of the words it is the prefix of
    TEST_ASSERT_TRUE(content.suggestionButtons.nbUsedButtons > 0);
    TEST_ASSERT_EQUAL_STRING(word, screen.keyboard.suggestionParams.buttons[0]);
    screen.keyboard.suggestionParams.onButtonCallback(
        screen.keyboard.suggestionParams.firstButtonToken, 0);
}

static void type_words(const char* const* words, size_t nb_words) {
    for (size_t i = 0; i < nb_words; i++) {
        TEST_ASSERT_EQUAL(i + 1, screen.keyboard.number);
        type_word(words[i]);
    }
}

static void type_passphrase(const char* passphrase) {
    TEST_ASSERT_EQUAL(UC_KEYBOARD, screen.use_case);
    TEST_ASSERT_EQUAL(KEYBOARD_WITH_BUTTON, screen.keyboard.type);
    TEST_ASSERT_FALSE(screen.keyboard.lettersOnly);
    TEST_ASSERT_EQUAL(MAX_PASSPHRASE_LENGTH, screen.keyboard.entryMaxLen);
    TEST_ASSERT_EQUAL_STRING("", screen.keyboard.entryBuffer);
    strcpy(screen.keyboard.entryBuffer, passphrase);
}

static void assert_result(bool success) {
    TEST_ASSERT_EQUAL(UC_RESULT, screen.use_case);
    TEST_ASSERT_EQUAL_PTR(
        success ? &C_Check_Circle_64px : &C_Denied_Circle_64px, screen.icon);
    TEST_ASSERT_NOT_NULL(
        strstr(screen.message, success ? "matches" : "doesn't match"));
    // nothing of the phrase is left behind
    TEST_ASSERT_EQUAL(0, get_current_word_number());
    TEST_ASSERT_EQUAL(0, get_mnemonic_passphrase_length());
}

static void assert_zeroed(const char* buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(0, buffer[i], "buffer not wiped");
    }
}

void setUp(void) {
    memset(&screen, 0, sizeof(screen));
    Mocklcx_hmac_Init();
    Mocklcx_pbkdf2_Init();
    Mocklcx_sha256_Init();
    Mockos_seed_Init();
    Mockos_utils_Init();
    Mocknbgl_use_case_Init();
    Mockmain_std_app_Init();
    fake_crypto_setup(REFERENCE_ROOT_KEY);
    nbgl_useCaseHomeAndSettings_StubWithCallback(fake_home_and_settings);
    nbgl_useCaseNavigableContent_StubWithCallback(fake_navigable_content);
    nbgl_useCaseKeyboard_StubWithCallback(fake_keyboard);
    nbgl_useCaseAction_StubWithCallback(fake_action);

    // the setting survives between tests, as between two checks
    ui_idle_init();
    if (passphrase_switch()->initState == ON_STATE) {
        toggle_passphrase();
        ui_idle_init();
    }
}

void tearDown(void) {
    Mocklcx_hmac_Verify();
    Mocklcx_pbkdf2_Verify();
    Mocklcx_sha256_Verify();
    Mockos_seed_Verify();
    Mockos_utils_Verify();
    Mocknbgl_use_case_Verify();
    Mockmain_std_app_Verify();
    Mocklcx_hmac_Destroy();
    Mocklcx_pbkdf2_Destroy();
    Mocklcx_sha256_Destroy();
    Mockos_seed_Destroy();
    Mockos_utils_Destroy();
    Mocknbgl_use_case_Destroy();
    Mockmain_std_app_Destroy();
}

static void test_home(void) {
    TEST_ASSERT_EQUAL(UC_HOME, screen.use_case);
    TEST_ASSERT_EQUAL_PTR(app_exit, screen.quit);
    TEST_ASSERT_EQUAL_STRING(
        "Select the number of words written on your Recovery Sheet",
        screen.action->text);
    TEST_ASSERT_EQUAL_STRING("Passphrase", passphrase_switch()->text);
    TEST_ASSERT_EQUAL(OFF_STATE, passphrase_switch()->initState);
}

static void test_passphrase_setting(void) {
    toggle_passphrase();
    TEST_ASSERT_EQUAL(ON_STATE, passphrase_switch()->initState);

    // kept when the home page is displayed again
    ui_idle_init();
    TEST_ASSERT_EQUAL(ON_STATE, passphrase_switch()->initState);

    toggle_passphrase();
    TEST_ASSERT_EQUAL(OFF_STATE, passphrase_switch()->initState);
    ui_idle_init();
    TEST_ASSERT_EQUAL(OFF_STATE, passphrase_switch()->initState);
}

static void test_length_choice(void) {
    const size_t lengths[] = {12, 18, 24};

    for (size_t i = 0; i < sizeof(lengths) / sizeof(lengths[0]); i++) {
        choose_length(lengths[i]);
        TEST_ASSERT_EQUAL(UC_KEYBOARD, screen.use_case);
        TEST_ASSERT_EQUAL(lengths[i], get_mnemonic_final_size());
        TEST_ASSERT_EQUAL(1, screen.keyboard.number);
        TEST_ASSERT_EQUAL_STRING("Enter word no. 1 from your Recovery Sheet",
                                 screen.keyboard.title);
        TEST_ASSERT_TRUE(screen.keyboard.lettersOnly);

        // back to the length choice, then to the home page
        screen.keyboard_back();
        TEST_ASSERT_EQUAL(UC_LENGTH_CHOICE, screen.use_case);
        screen.length_back();
        TEST_ASSERT_EQUAL(UC_HOME, screen.use_case);
    }
}

static void test_suggestions(void) {
    nbgl_layoutKeyboardContent_t content = {0};
    uint32_t mask = 0;

    choose_length(12);
    char* entry = screen.keyboard.entryBuffer;

    // no suggestion before 2 letters, but the keys are filtered
    strcpy(entry, "q");
    screen.keyboard.suggestionParams.updateButtonsCallback(&content, &mask);
    TEST_ASSERT_EQUAL(0, content.suggestionButtons.nbUsedButtons);
    TEST_ASSERT_EQUAL(1, content.number);
    TEST_ASSERT_NOT_EQUAL(0, mask);

    strcpy(entry, "ab");
    mask = 0;
    screen.keyboard.suggestionParams.updateButtonsCallback(&content, &mask);
    TEST_ASSERT_EQUAL(NB_MAX_SUGGESTION_BUTTONS,
                      content.suggestionButtons.nbUsedButtons);
    TEST_ASSERT_EQUAL(10, content.suggestionButtons.nbCandidates);
    TEST_ASSERT_EQUAL_STRING("abandon",
                             screen.keyboard.suggestionParams.buttons[0]);
    TEST_ASSERT_NOT_EQUAL(0, mask);

    // back to 1 letter: the suggestions are dropped
    const char* candidates = screen.keyboard.suggestionParams.buttons[0];
    strcpy(entry, "a");
    screen.keyboard.suggestionParams.updateButtonsCallback(&content, &mask);
    TEST_ASSERT_EQUAL(0, content.suggestionButtons.nbUsedButtons);
    TEST_ASSERT_EQUAL(0, content.suggestionButtons.nbCandidates);
    TEST_ASSERT_NULL(screen.keyboard.suggestionParams.buttons[0]);
    assert_zeroed(candidates, MAX_WORD_LENGTH + 1);

    // empty entry: the mask is left untouched
    entry[0] = '\0';
    mask = 0x1234;
    screen.keyboard.suggestionParams.updateButtonsCallback(&content, &mask);
    TEST_ASSERT_EQUAL_HEX32(0x1234, mask);
}

static void test_selected_word_wipes_the_buffers(void) {
    choose_length(12);
    char* entry = screen.keyboard.entryBuffer;

    type_word("abandon");
    // the next word starts with clean buffers
    TEST_ASSERT_EQUAL(UC_KEYBOARD, screen.use_case);
    TEST_ASSERT_EQUAL(2, screen.keyboard.number);
    TEST_ASSERT_EQUAL_STRING("Enter word no. 2 from your Recovery Sheet",
                             screen.keyboard.title);
    assert_zeroed(entry, MAX_WORD_LENGTH + 1);
    TEST_ASSERT_NULL(screen.keyboard.suggestionParams.buttons[0]);
    TEST_ASSERT_EQUAL_STRING("abandon", get_mnemonic());
}

static void test_previous_word(void) {
    choose_length(24);
    type_word("glory");
    type_word("promote");
    TEST_ASSERT_EQUAL(3, screen.keyboard.number);

    // the prefix being typed is wiped, the last word removed
    strcpy(screen.keyboard.entryBuffer, "ma");
    char* entry = screen.keyboard.entryBuffer;
    screen.keyboard_back();
    TEST_ASSERT_EQUAL(UC_KEYBOARD, screen.use_case);
    TEST_ASSERT_EQUAL(2, screen.keyboard.number);
    TEST_ASSERT_EQUAL_STRING("glory", get_mnemonic());
    assert_zeroed(entry, MAX_WORD_LENGTH + 1);

    screen.keyboard_back();
    TEST_ASSERT_EQUAL(1, screen.keyboard.number);
    TEST_ASSERT_EQUAL_STRING("", get_mnemonic());

    // no word left: back to the length choice
    screen.keyboard_back();
    TEST_ASSERT_EQUAL(UC_LENGTH_CHOICE, screen.use_case);
}

static void test_check_ok(void) {
    choose_length(24);
    type_words(SPECULOS_MNEMONIC, SPECULOS_MNEMONIC_SIZE);
    assert_result(true);

    screen.close();
    TEST_ASSERT_EQUAL(UC_HOME, screen.use_case);
}

static void test_check_wrong(void) {
    const char* const words[] = {"abandon", "abandon", "abandon", "abandon",
                                 "abandon", "abandon", "abandon", "abandon",
                                 "abandon", "abandon", "abandon", "about"};

    choose_length(12);
    type_words(words, sizeof(words) / sizeof(words[0]));
    assert_result(false);
}

static void test_passphrase_ok(void) {
    fake_crypto_setup(REFERENCE_PASSPHRASE_ROOT_KEY);
    toggle_passphrase();
    choose_length(24);
    type_words(SPECULOS_MNEMONIC, SPECULOS_MNEMONIC_SIZE);

    // the passphrase is asked after the last word
    type_passphrase(REFERENCE_PASSPHRASE);
    char* entry = screen.keyboard.entryBuffer;
    screen.keyboard.confirmationParams.onButtonCallback();
    assert_result(true);
    assert_zeroed(entry, MAX_PASSPHRASE_LENGTH + 1);
}

static void test_passphrase_wrong(void) {
    fake_crypto_setup(REFERENCE_PASSPHRASE_ROOT_KEY);
    toggle_passphrase();
    choose_length(24);
    type_words(SPECULOS_MNEMONIC, SPECULOS_MNEMONIC_SIZE);

    type_passphrase("Ledger 25th word");
    screen.keyboard.confirmationParams.onButtonCallback();
    assert_result(false);
}

static void test_passphrase_not_asked_when_disabled(void) {
    // device unlocked with a passphrase, setting disabled: words only
    fake_crypto_setup(REFERENCE_PASSPHRASE_ROOT_KEY);
    choose_length(24);
    type_words(SPECULOS_MNEMONIC, SPECULOS_MNEMONIC_SIZE);
    assert_result(false);
}

static void test_passphrase_back_to_last_word(void) {
    toggle_passphrase();
    choose_length(24);
    type_words(SPECULOS_MNEMONIC, SPECULOS_MNEMONIC_SIZE);

    type_passphrase("typed");
    char* entry = screen.keyboard.entryBuffer;
    screen.keyboard_back();
    assert_zeroed(entry, MAX_PASSPHRASE_LENGTH + 1);
    TEST_ASSERT_EQUAL(KEYBOARD_WITH_SUGGESTIONS, screen.keyboard.type);
    TEST_ASSERT_EQUAL(24, screen.keyboard.number);
    TEST_ASSERT_EQUAL(23, get_current_word_number());

    // the last word again, then an empty passphrase: the plain seed
    type_word("skin");
    type_passphrase("");
    screen.keyboard.confirmationParams.onButtonCallback();
    assert_result(true);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_home);
    RUN_TEST(test_passphrase_setting);
    RUN_TEST(test_length_choice);
    RUN_TEST(test_suggestions);
    RUN_TEST(test_selected_word_wipes_the_buffers);
    RUN_TEST(test_previous_word);
    RUN_TEST(test_check_ok);
    RUN_TEST(test_check_wrong);
    RUN_TEST(test_passphrase_ok);
    RUN_TEST(test_passphrase_wrong);
    RUN_TEST(test_passphrase_not_asked_when_disabled);
    RUN_TEST(test_passphrase_back_to_last_word);
    return UNITY_END();
}
