#include <string.h>
#include <os.h>

#include "constants.h"
#include "glyphs.h"
#include "main_std_app.h"

#if defined(HAVE_NBGL)

#include "nbgl_use_case.h"
#include "ui.h"
#include "bip39.h"
#include "mnemonic.h"

#define HEADER_SIZE 50

// Keyboard UI variables
static nbgl_layout_t *layout = 0;
static int keyboardIndex = 0;
static char headerText[HEADER_SIZE] = {0};
static char textToEnter[MAX_WORD_LENGTH + 1] = {0};
// the biggest word of BIP39 list is 8 char (9 with trailing '\0'), and
// the max number of showed suggestions is NB_MAX_SUGGESTION_BUTTONS
static char wordCandidates[(MAX_WORD_LENGTH + 1) * NB_MAX_SUGGESTION_BUTTONS] = {0};

static nbgl_layoutSuggestionButtons_t suggestionButtons = {0};
static nbgl_layoutKeyboardContent_t keyboardContent = {0};

// Suggestion button texts
static const char *buttonTexts[NB_MAX_SUGGESTION_BUTTONS] = {0};

// Buttons tokens
enum {
    BACK_BUTTON_TOKEN = FIRST_USER_TOKEN,
    CHOOSE_MNEMONIC_SIZE_TOKEN,
    FIRST_SUGGESTION_TOKEN,
    KBD_TEXT_TOKEN,
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

/**
 * @brief Reset the current contexts
 *
 */
static void reset_globals(void) {
    reset_mnemonic();
    memset(buttonTexts, 0, sizeof(buttonTexts[0]) * NB_MAX_SUGGESTION_BUTTONS);
}

/**
 * @brief Passphrase callback
 *
 * @param[in] token of the widget on the page
 * @param[out] index of the activated radio button
 * @return true if the navigation was successful, false otherwise
 *
 */
static void passphrase_callback(int token, uint8_t index) {
    if (token != CHOOSE_MNEMONIC_SIZE_TOKEN) {
        return;
    }
#ifdef HAVE_PIEZO_SOUND
    io_seproxyhal_play_tune(TUNE_TAP_CASUAL);
#endif
    switch (index) {
        case BUTTON_12_INDEX:
            set_mnemonic_final_size(MNEMONIC_SIZE_12);
            break;
        case BUTTON_18_INDEX:
            set_mnemonic_final_size(MNEMONIC_SIZE_18);
            break;
        case BUTTON_24_INDEX:
            set_mnemonic_final_size(MNEMONIC_SIZE_24);
            break;
        default:
            break;
    }
    nbgl_layoutRelease(layout);
    display_keyboard_page();
}

/**
 * @brief Passphrase navigation callback
 *
 * @param[in] page index of the page
 * @param[out] content pointer to the content structure
 * @return true if the navigation was successful, false otherwise
 *
 */
static bool passphrase_choice_callback(const uint8_t page, nbgl_pageContent_t *content) {
    UNUSED(page);
    content->type = CHOICES_LIST;
    content->choicesList.names = passphraseLength;
    content->choicesList.token = CHOOSE_MNEMONIC_SIZE_TOKEN;
    content->choicesList.initChoice = BUTTON_24_INDEX;
    content->choicesList.nbChoices = NB_BUTTONS;
    return true;
}

/**
 * @brief Passphrase length selection page
 *
 */
static void passphrase_length_page(void) {
    nbgl_useCaseNavigableContent("How long is your Recovery Phrase?",
                                 0,
                                 1,
                                 display_home_page,
                                 passphrase_choice_callback,
                                 passphrase_callback);
}

/**
 * @brief Keyboard control callback
 *
 * @param[in] token button Id pressed
 * @param[in] index widget index on the page
 *
 */
static void keyboard_dispatcher(const int token, uint8_t index) {
    UNUSED(index);
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

/**
 * @brief Keyboard press callback
 *
 * @param[in] touchedKey key pressed
 *
 */
static void key_press_callback(const char touchedKey) {
    uint32_t mask = 0;
    // Update word currently displayed
    size_t textLen = strlen(textToEnter);
    if (touchedKey == BACKSPACE_KEY) {
        if (textLen == 0) {
            return;
        }
        textToEnter[--textLen] = '\0';
    } else {
        textToEnter[textLen] = touchedKey;
        textToEnter[++textLen] = '\0';
    }
    PRINTF("Current text is: '%s' (size '%d')\n", textToEnter, textLen);

    // Update the screen (written word, suggestions, ...)
    keyboardContent.number = get_current_word_number() + 1;

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

    // Add keyboard
    keyboardIndex = nbgl_layoutAddKeyboard(layout, &kbdInfo);
    if (keyboardIndex < 0) {
        // Error
        nbgl_layoutRelease(layout);
        return;
    }

    nbgl_layoutAddKeyboardContent(layout, &keyboardContent);
    nbgl_layoutDraw(layout);
    nbgl_refresh();
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
        INIT_HOME_PAGE,
        NULL,
        &infoList,
        &action,
        app_exit);
}

/**
 * @brief Display the Result page
 *
 * @param[in] result True if the recovery phrase is valid, false otherwise
 *
 */
static void display_result_page(const bool result) {
    reset_globals();

    nbgl_useCaseAction(icons[result],
                       possible_results[result],
                       "Close",
                       display_home_page);
}

/*
 * Public function
 */

/**
 * @brief UI initialization
 *
 */
void ui_idle_init(void) {
    display_home_page();
}

#endif
