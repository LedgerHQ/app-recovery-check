#include <string.h>
#include <os.h>

#include "constants.h"
#include "glyphs.h"

#if defined(HAVE_NBGL)

#include "nbgl_use_case.h"
#include "ui.h"
#include "bip39.h"
#include "mnemonic.h"
#include "passphrase_length_screen.h"

#define HEADER_SIZE 50

static nbgl_page_t *pageContext;

static char headerText[HEADER_SIZE] = {0};
static nbgl_layout_t *layout = 0;

// Suggestion button texts
static const char *buttonTexts[NB_MAX_SUGGESTION_BUTTONS] = {0};

enum {
    BACK_BUTTON_TOKEN = FIRST_USER_TOKEN,
    CHOOSE_MNEMONIC_SIZE_TOKEN,
    FIRST_SUGGESTION_TOKEN,
    START_RECOVER_TOKEN,
    RESULT_TOKEN,
};

// Mnemonic size
enum {
    BUTTON_12_INDEX,
    BUTTON_18_INDEX,
    BUTTON_24_INDEX,
    NB_BUTTONS
};
static const char* const passphraseLength[NB_BUTTONS] = {"12 words", "18 words", "24 words"};

// Result page
static const char *possible_results[2] = {
    "The Recovery Phrase you have entered doesn't match the one present on this device!",
    "The Recovery Phrase you have entered matches the one present on this device."
};
static const nbgl_icon_details_t *icons[2] = {&WARNING_ICON, &ICON_SUCCESS};

static void display_keyboard_page(void);
static void display_home_page(void);
static void display_result_page(const bool result);

// Home page, infos & dispatcher
#define NB_INFOS 2
static const char *const infoTypes[NB_INFOS] = {"Version", "Recovery Check"};
static const char *const infoContents[NB_INFOS] = {APPVERSION, "(c) 2018-2025 Ledger"};

static const nbgl_contentInfoList_t infoList = {
    .nbInfos = NB_INFOS,
    .infoTypes = infoTypes,
    .infoContents = infoContents,
};

static nbgl_homeAction_t action = {0};

/*
 * Utils
 */
static const char *buttonTexts[NB_MAX_SUGGESTION_BUTTONS] = {0};

static void reset_globals() {
    reset_mnemonic();
    memset(buttonTexts, 0, sizeof(buttonTexts[0]) * NB_MAX_SUGGESTION_BUTTONS);
}

static void on_quit(void) {
    os_sched_exit(-1);
}

/*
 * Choose mnemonic size page
 */
enum {
    ICON_INDEX = 0,
    TEXT_INDEX,
    BUTTON_12_INDEX,
    BUTTON_18_INDEX,
    BUTTON_24_INDEX,
    BACK_BUTTON_INDEX,
    KBD_TEXT_TOKEN,
    NB_CHILDREN
};

#define NB_BUTTONS 3

static const char *passphraseLength[] = {"12 words", "18 words", "24 words"};
static void passphrase_length_callback(nbgl_obj_t *obj, nbgl_touchType_t eventType) {
    nbgl_obj_t **screenChildren = nbgl_screenGetElements(0);
    if (eventType != TOUCHED) {
        return;
    }
    io_seproxyhal_play_tune(TUNE_TAP_CASUAL);
    if (obj == screenChildren[BUTTON_12_INDEX]) {
        set_mnemonic_final_size(MNEMONIC_SIZE_12);
    } else if (obj == screenChildren[BUTTON_18_INDEX]) {
        set_mnemonic_final_size(MNEMONIC_SIZE_18);
    } else if (obj == screenChildren[BUTTON_24_INDEX]) {
        set_mnemonic_final_size(MNEMONIC_SIZE_24);
    } else if (obj == screenChildren[BACK_BUTTON_INDEX]) {
        nbgl_layoutRelease(layout);
        display_home_page();
        return;
    }
    nbgl_layoutRelease(layout);
    display_keyboard_page();
}

static void passphrase_length_page(void) {
    nbgl_obj_t **screenChildren;

    // From top to bottom:
    // <return back arrow> + <icon> + <text> + <3 buttons>
    nbgl_screenSet(&screenChildren, 6, NULL, (nbgl_touchCallback_t) &passphrase_length_callback);

    screenChildren[ICON_INDEX] = (nbgl_obj_t *) passphrase_length_set_icon();
    screenChildren[TEXT_INDEX] =
        (nbgl_obj_t *) passphrase_length_set_title(screenChildren[ICON_INDEX]);

    // create nb words buttons
    nbgl_objPoolGetArray(BUTTON, NB_BUTTONS, 0, (nbgl_obj_t **) &screenChildren[BUTTON_12_INDEX]);
    passphrase_length_configure_buttons((nbgl_button_t **) &screenChildren[BUTTON_12_INDEX],
                                        NB_BUTTONS);
    ((nbgl_button_t *) screenChildren[BUTTON_12_INDEX])->text = passphraseLength[0];
    ((nbgl_button_t *) screenChildren[BUTTON_18_INDEX])->text = passphraseLength[1];
    ((nbgl_button_t *) screenChildren[BUTTON_24_INDEX])->text = passphraseLength[2];
    ((nbgl_button_t *) screenChildren[BUTTON_24_INDEX])->borderColor = BLACK;
    ((nbgl_button_t *) screenChildren[BUTTON_24_INDEX])->innerColor = BLACK;
    ((nbgl_button_t *) screenChildren[BUTTON_24_INDEX])->foregroundColor = WHITE;

    // create back button
    screenChildren[BACK_BUTTON_INDEX] = (nbgl_obj_t *) passphrase_length_set_back_button();

    nbgl_screenRedraw();
}

/*
 * Word recover page
 */
#define BUTTON_VMARGIN 32

static char textToEnter[MAX_WORD_LENGTH + 1] = {0};
static int keyboardIndex = 0;
// the biggest word of BIP39 list is 8 char (9 with trailing '\0'), and
// the max number of showed suggestions is NB_MAX_SUGGESTION_BUTTONS
static char wordCandidates[(MAX_WORD_LENGTH + 1) * NB_MAX_SUGGESTION_BUTTONS] = {0};

static void keyboard_dispatcher(const int token, uint8_t index __attribute__((unused))) {
    if (token == BACK_BUTTON_TOKEN) {
        nbgl_layoutRelease(layout);
        if (remove_word_from_mnemonic()) {
            display_keyboard_page();
        } else {
            passphrase_length_page();
        }
    } else if (token >= FIRST_SUGGESTION_TOKEN) {
        nbgl_layoutRelease(layout);
        PRINTF("Selected word is '%s' (size '%d')\n",
               buttonTexts[token - FIRST_SUGGESTION_TOKEN],
               strlen(buttonTexts[token - FIRST_SUGGESTION_TOKEN]));
        add_word_in_mnemonic(buttonTexts[token - FIRST_SUGGESTION_TOKEN],
                             strlen(buttonTexts[token - FIRST_SUGGESTION_TOKEN]));
        if (is_mnemonic_complete()) {
            display_result_page(check_mnemonic());
        } else {
            display_keyboard_page();
        }
    }
}

// function called when a key of keyboard is touched
static void key_press_callback(const char touchedKey) {
    size_t textLen = 0;
    uint32_t mask = 0;
    // Update word currently displayed
    const size_t previousTextLen = strlen(textToEnter);
    if (touchedKey == BACKSPACE_KEY) {
        if (previousTextLen == 0) {
            return;
        }
        textToEnter[previousTextLen - 1] = '\0';
        textLen = previousTextLen - 1;
    } else {
        textToEnter[previousTextLen] = touchedKey;
        textToEnter[previousTextLen + 1] = '\0';
        textLen = previousTextLen + 1;
    }

    // Update the screen (written word, suggestions, ...)
    nbgl_layoutSuggestionButtons_t suggestionButtons = {
        .buttons = PIC(buttonTexts),
        .firstButtonToken = FIRST_SUGGESTION_TOKEN,
        .nbUsedButtons = 0,
    };
    nbgl_layoutKeyboardContent_t keyboardContent = {
        .type = KEYBOARD_WITH_SUGGESTIONS,
        .title = PIC(headerText),
        .text = PIC(textToEnter),
        .numbered = true,
        .number = get_current_word_number() + 1,
        .grayedOut = false,
        .textToken = KBD_TEXT_TOKEN,
        .suggestionButtons = suggestionButtons,
        .tuneId = TUNE_TAP_CASUAL,
    };
    PRINTF("Current text is: '%s' (size '%d')\n", textToEnter, textLen);

    if (textLen < 2) {
        // Suggestions only when the word contains 2+ letters
        nbgl_layoutUpdateKeyboardContent(layout, &keyboardContent);
    } else {
        const size_t nbMatchingWords =
            bolos_ux_bip39_fill_with_candidates((unsigned char *) &(textToEnter[0]),
                                                strlen(textToEnter),
                                                wordCandidates,
                                                buttonTexts);
        keyboardContent.suggestionButtons.nbUsedButtons = nbMatchingWords;
        nbgl_layoutUpdateKeyboardContent(layout, &keyboardContent);
    }
    if (textLen > 0) {
        mask = bolos_ux_bip39_get_keyboard_mask((unsigned char *) &(textToEnter[0]),
                                                strlen(textToEnter));
    }
    nbgl_layoutUpdateKeyboard(layout, keyboardIndex, mask, false, LOWER_CASE);
    nbgl_refreshSpecialWithPostRefresh(BLACK_AND_WHITE_REFRESH, POST_REFRESH_FORCE_POWER_ON);
}

/**
 * @brief Display the passwords creation page
 *
 */
static void display_keyboard_page(void) {
    nbgl_layoutDescription_t layoutDescription = {0};
    nbgl_layoutKbd_t kbdInfo = {
        .callback = &key_press_callback,
    };
    nbgl_layoutHeader_t headerDesc = {
        .type = HEADER_BACK_AND_TEXT,
        .backAndText.token = BACK_BUTTON_TOKEN,
#ifdef HAVE_PIEZO_SOUND
        .backAndText.tuneId = TUNE_TAP_CASUAL,
#endif
    };
    suggestionButtons = (nbgl_layoutSuggestionButtons_t) {
        .buttons = PIC(buttonTexts),
        .firstButtonToken = FIRST_SUGGESTION_TOKEN,
    };
    keyboardContent = (nbgl_layoutKeyboardContent_t){
        .type = KEYBOARD_WITH_SUGGESTIONS,
        .title = PIC(headerText),
        .text = PIC(textToEnter),
        .numbered = true,
        .number = get_current_word_number() + 1,
        .textToken = KBD_TEXT_TOKEN,
        .suggestionButtons = suggestionButtons,
#ifdef HAVE_PIEZO_SOUND
        .tuneId = TUNE_TAP_CASUAL,
#endif
    };
    layoutDescription.onActionCallback = &keyboard_dispatcher;
    kbdInfo.mode = MODE_LETTERS;
    kbdInfo.lettersOnly = true;
    textToEnter[0] = '\0';
    memset(buttonTexts, 0, sizeof(buttonTexts[0]) * NB_MAX_SUGGESTION_BUTTONS);

    snprintf(headerText,
             HEADER_SIZE,
             "Enter word n. %d/%d from your Recovery Sheet",
             get_current_word_number() + 1,
             get_mnemonic_final_size());

    // Create page layout
    layout = nbgl_layoutGet(&layoutDescription);

    // Add header
    nbgl_layoutAddHeader(layout, &headerDesc);

    keyboardIndex = nbgl_layoutAddKeyboard(layout, &kbdInfo);

    nbgl_layoutSuggestionButtons_t suggestionButtons = {
        .buttons = PIC(buttonTexts),
        .firstButtonToken = FIRST_SUGGESTION_TOKEN,
        .nbUsedButtons = 0,
    };
    nbgl_layoutKeyboardContent_t keyboardContent = {
        .type = KEYBOARD_WITH_SUGGESTIONS,
        .title = PIC(headerText),
        .text = PIC(textToEnter),
        .numbered = true,
        .number = get_current_word_number() + 1,
        .grayedOut = false,
        .textToken = KBD_TEXT_TOKEN,
        .suggestionButtons = suggestionButtons,
        .tuneId = TUNE_TAP_CASUAL,
    };
    nbgl_layoutAddKeyboardContent(layout, &keyboardContent);
    nbgl_layoutDraw(layout);
}

/**
 * @brief Display the App Home page
 *
 */
static void display_home_page(void) {
    reset_globals();

    action.callback = (nbgl_callback_t) passphrase_length_page;
    action.text = "Start check";

    nbgl_useCaseHomeAndSettings(
        APPNAME,
        &ICON_APP_HOME,
        "Enter a Recovery Phrase and test if it matches "
        "the one present on this device",
#endif
        INIT_HOME_PAGE,
        NULL,
        &infoList,
        &action,
        on_quit);
}

#if defined(TARGET_STAX)
#define DEVICE "Ledger Stax"
#elif defined(TARGET_FLEX)
#define DEVICE "Ledger Flex"
#endif

/*
 * Result page
 */
static const char *possible_results[2][2] = {
    {"Incorrect Secret\nRecovery Phrase",
     "The Recovery Phrase you have\nentered doesn't match the one\npresent on this " DEVICE "."},
    {"Correct Secret\nRecovery Phrase",
     "The Recovery Phrase you have\nentered matches the one\npresent on this " DEVICE "."}};
static const nbgl_icon_details_t *icons[2] = {&WARNING_ICON, &VALIDATE_ICON};

static void result_callback(int token __attribute__((unused)),
                            uint8_t index __attribute__((unused))) {
    display_home_page();
}

static void display_result_page(const bool result) {
    reset_globals();
    nbgl_pageInfoDescription_t info = {.centeredInfo.icon = icons[result],
                                       .centeredInfo.text1 = possible_results[result][0],
                                       .centeredInfo.text2 = possible_results[result][1],
                                       .centeredInfo.text3 = NULL,
                                       .centeredInfo.style = LARGE_CASE_INFO,
                                       .centeredInfo.offsetY = -16,
                                       .footerText = "Tap to dismiss",
                                       .footerToken = RESULT_TOKEN,
                                       .bottomButtonStyle = NO_BUTTON_STYLE,
                                       .tapActionText = NULL,
                                       .topRightStyle = NO_BUTTON_STYLE,
                                       .actionButtonText = NULL,
                                       .tuneId = TUNE_TAP_CASUAL};
    pageContext = nbgl_pageDrawInfo(&result_callback, NULL, &info);
    nbgl_refresh();
}

/*
 * Public function
 */
void ui_idle_init(void) {
    display_home_page();
}

#endif
