#include <string.h>
#include <os.h>

#include "constants.h"
#include "glyphs.h"
#include "main_std_app.h"

#include "nbgl_use_case.h"
#include "ui.h"
#include "bip39.h"
#include "mnemonic.h"

#define HEADER_SIZE 50

// Keyboard UI variables
static char headerText[HEADER_SIZE] = {0};
static char textToEnter[MAX_WORD_LENGTH + 1] = {0};
#ifdef SCREEN_SIZE_WALLET
// the biggest word of BIP39 list is 8 char (9 with trailing '\0'), and
// the max number of showed suggestions is NB_MAX_SUGGESTION_BUTTONS
static char wordCandidates[(MAX_WORD_LENGTH + 1) * NB_MAX_SUGGESTION_BUTTONS] = {0};

// Suggestion button texts
static const char *buttonTexts[NB_MAX_SUGGESTION_BUTTONS] = {0};
#else
static int textIndex = 0;
#endif

// Buttons tokens
enum {
    BACK_BUTTON_TOKEN = FIRST_USER_TOKEN,
    BUTTON_12_TOKEN,
    BUTTON_18_TOKEN,
    BUTTON_24_TOKEN,
    KBD_TEXT_TOKEN,
    FIRST_SUGGESTION_TOKEN,
};

// Mnemonic size
enum { BUTTON_12_INDEX, BUTTON_18_INDEX, BUTTON_24_INDEX, NB_BUTTONS };
static const char *const passphraseLength[NB_BUTTONS] = {"12 words", "18 words", "24 words"};
static const uint8_t passphraseTokens[NB_BUTTONS] = {BUTTON_12_TOKEN,
                                                     BUTTON_18_TOKEN,
                                                     BUTTON_24_TOKEN};

// Result page
static const char *possible_results[2] = {
#ifdef SCREEN_SIZE_WALLET
    "The Phrase you have entered doesn't match the one present on this Ledger device.",
    "The Phrase you have entered matches the one present on this Ledger device."
#else
    "Invalid Recovery Phrase!",
    "Successful Recovery Phrase."
#endif
};
static const nbgl_icon_details_t *icons[2] = {&ICON_FAIL, &ICON_SUCCESS};

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

/**
 * @brief Reset the current contexts
 *
 */
static void reset_globals(void) {
    reset_mnemonic();
#ifdef SCREEN_SIZE_WALLET
    memset(buttonTexts, 0, sizeof(buttonTexts[0]) * NB_MAX_SUGGESTION_BUTTONS);
#endif
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
    UNUSED(index);
    switch (token) {
        case BUTTON_12_TOKEN:
            set_mnemonic_final_size(MNEMONIC_SIZE_12);
            break;
        case BUTTON_18_TOKEN:
            set_mnemonic_final_size(MNEMONIC_SIZE_18);
            break;
        case BUTTON_24_TOKEN:
            set_mnemonic_final_size(MNEMONIC_SIZE_24);
            break;
        default:
            break;
    }
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
    content->type = BARS_LIST;
    content->barsList.barTexts = passphraseLength;
    content->barsList.tokens = passphraseTokens;
    content->barsList.nbBars = NB_BUTTONS;
#ifdef HAVE_PIEZO_SOUND
    content->barsList.tuneId = TUNE_TAP_CASUAL;
#endif
    return true;
}

/**
 * @brief Passphrase length selection page
 *
 */
static void passphrase_length_page(void) {
    nbgl_useCaseNavigableContent(
#ifdef SCREEN_SIZE_WALLET
        "How long is your Recovery Phrase?",
#else
        "Phrase Length?",
#endif
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

    if (token >= FIRST_SUGGESTION_TOKEN) {
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
static void update_buttons_callback(nbgl_layoutKeyboardContent_t *content, uint32_t *mask) {
    size_t textLen = strlen(textToEnter);
    // Update the screen (written word, suggestions, ...)
    content->number = get_current_word_number() + 1;

    if (textLen < 2) {
        // Suggestions only when the word contains 2+ letters
        content->suggestionButtons.nbUsedButtons = 0;
    } else {
        const size_t nbMatchingWords =
            bolos_ux_bip39_fill_with_candidates((unsigned char *) &(textToEnter[0]),
                                                strlen(textToEnter),
                                                wordCandidates,
                                                buttonTexts);
        content->suggestionButtons.nbUsedButtons = nbMatchingWords;
    }
    if (textLen > 0) {
        *mask = bolos_ux_bip39_get_keyboard_mask((unsigned char *) &(textToEnter[0]),
                                                 strlen(textToEnter));
    }
}

/**
 * @brief Keyboard control callback
 *
 */
static void keyboard_close(void) {
    if (remove_word_from_mnemonic()) {
        display_keyboard_page();
    } else {
        passphrase_length_page();
    }
}

/**
 * @brief Display the passwords creation page
 *
 */
static void display_keyboard_page(void) {
    nbgl_kbdSuggestParams_t suggestParams = {
        .buttons = buttonTexts,
        .firstButtonToken = FIRST_SUGGESTION_TOKEN,
        .onButtonCallback = &keyboard_dispatcher,
        .updateButtonsCallback = &update_buttons_callback,
    };
    nbgl_keyboardParams_t keyboardParams = {
        .type = KEYBOARD_WITH_SUGGESTIONS,
        .title = headerText,
        .entryBuffer = textToEnter,
        .entryMaxLen = sizeof(textToEnter),
        .numbered = true,
        .number = get_current_word_number() + 1,
        .lettersOnly = true,
        .mode = MODE_LETTERS,
        .casing = LOWER_CASE,
        .suggestionParams = suggestParams,
    };

    textToEnter[0] = '\0';
    memset(buttonTexts, 0, sizeof(buttonTexts[0]) * NB_MAX_SUGGESTION_BUTTONS);
    snprintf(headerText,
             HEADER_SIZE,
             "Enter word no. %d from your Recovery Sheet",
             get_current_word_number() + 1);

    nbgl_useCaseKeyboard(&keyboardParams, &keyboard_close);
}

/**
 * @brief Display the App Home page
 *
 */
static void display_home_page(void) {
    reset_globals();

    action.callback = (nbgl_callback_t) passphrase_length_page;
    action.text = "Start check";

    nbgl_useCaseHomeAndSettings(APPNAME,
                                &ICON_APP_HOME,
#ifdef SCREEN_SIZE_WALLET
                                "Enter a Secret Recovery Phrase and test if it matches "
                                "the one present on this device",
#else
                                "Check your Recovery Phrase",
#endif
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

    nbgl_useCaseAction(icons[result], possible_results[result], "Close", display_home_page);
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
