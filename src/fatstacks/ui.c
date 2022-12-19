#include <string.h>
#include <os.h>

#include "constants.h"
#include "glyphs.h"

#if defined(HAVE_NBGL)

#include <nbgl_use_case.h>
#include <nbgl_fonts.h>
#include <nbgl_front.h>
#include <nbgl_debug.h>
#include <nbgl_page.h>
#include <nbgl_layout.h>

#include "../ux_common/common_bip39.h"
#include "ui.h"
#include "ux_fatstacks.h"

#define HEADER_SIZE 50

static nbgl_page_t *pageContext;
static char headerText[HEADER_SIZE] = {0};
static nbgl_layout_t *layout = 0;

static void display_keyboard_page(void);
static void display_home_page(void);
static void display_result_page(const bool result);
static void display_mnemonic_page(void);
static void reset_globals(void);
static bool onInfos(uint8_t page, nbgl_pageContent_t *content);

enum {
    BACK_BUTTON_TOKEN = 0,
    CHOOSE_MNEMONIC_SIZE_TOKEN,
    FIRST_SUGGESTION_TOKEN,
    START_RECOVER_TOKEN,
};

static char *buttonTexts[NB_MAX_SUGGESTION_BUTTONS] = {0};

/*
 * Utils
 */
static void reset_globals() {
    reset_mnemonic();
    memset(buttonTexts, 0, NB_MAX_SUGGESTION_BUTTONS);
}


static void releaseContext(void) {
    if (pageContext != NULL) {
        nbgl_pageRelease(pageContext);
        pageContext = NULL;
    }
}

static void onHome(void) {
    releaseContext();
    ui_idle_init();
}

static void onQuit(void) {
    releaseContext();
    os_sched_exit(-1);
}

/*
 * About menu
 */
static const char *const infoTypes[] = {"Version", "Recovery Check"};
static const char *const infoContents[] = {APPVERSION, "(c) 2022 Ledger"};

static bool onInfos(uint8_t page, nbgl_pageContent_t *content) {
    if (page == 0) {
        content->type = INFOS_LIST;
        content->infosList.nbInfos = 2;
        content->infosList.infoTypes = (const char **) infoTypes;
        content->infosList.infoContents = (const char **) infoContents;
    } else {
        return false;
    }
    return true;
}

/*
 * Choose mnemonic size page
 */
static void mnemonic_dispatcher(const int token, uint8_t index) {
    if (token == BACK_BUTTON_TOKEN) {
        nbgl_layoutRelease(layout);
        display_home_page();
    } else if (token == CHOOSE_MNEMONIC_SIZE_TOKEN) {
        switch (index) {
            case 0:
                set_mnemonic_final_size(MNEMONIC_SIZE_12);
                break;
            case 1:
                set_mnemonic_final_size(MNEMONIC_SIZE_18);
                break;
            case 2:
                set_mnemonic_final_size(MNEMONIC_SIZE_24);
                break;
            default:
                PRINTF("Unexpected index '%d' (max 3)\n", index);
                nbgl_layoutRelease(layout);
                display_home_page();
                return;
        }
        nbgl_layoutRelease(layout);
        display_keyboard_page();
    }
}

static void display_mnemonic_page() {
    reset_globals();
    nbgl_layoutDescription_t layoutDescription = {.modal = false,
                                                  .onActionCallback = mnemonic_dispatcher};
    nbgl_layoutRadioChoice_t choices = {.names = (char *[]){"12 words", "18 words", "24 words"},
                                        .localized = false,
                                        .nbChoices = 3,
                                        .initChoice = 2,
                                        .token = CHOOSE_MNEMONIC_SIZE_TOKEN};
    nbgl_layoutCenteredInfo_t centeredInfo = {.text1 = NULL,
                                              .text2 = headerText,  // to use as "header"
                                              .text3 = NULL,
                                              .style = LARGE_CASE_INFO,
                                              .icon = NULL,
                                              .offsetY = 0,
                                              .onTop = true};
    layout = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddProgressIndicator(layout, 0, 0, true, BACK_BUTTON_TOKEN, TUNE_TAP_CASUAL);
    memset(headerText, 0, HEADER_SIZE);
    snprintf(headerText, HEADER_SIZE, "What is the length of your\nrecovery passphrase?");
    nbgl_layoutAddCenteredInfo(layout, &centeredInfo);
    nbgl_layoutAddRadioChoice(layout, &choices);
    nbgl_layoutDraw(layout);
}

/*
 * Word recover page
 */
static char textToEnter[MAX_WORD_LENGTH + 1] = {0};
static int textIndex, suggestionIndex, keyboardIndex = 0;
// the biggest word of BIP39 list is 8 char (9 with trailing '\0'), and
// the max number of showed suggestions is NB_MAX_SUGGESTION_BUTTONS
static char wordCandidates[(MAX_WORD_LENGTH + 1) * NB_MAX_SUGGESTION_BUTTONS] = {0};

static void keyboard_dispatcher(const int token, uint8_t index __attribute__((unused))) {
    if (token == BACK_BUTTON_TOKEN) {
        nbgl_layoutRelease(layout);
        if (remove_word_from_mnemonic()) {
            display_keyboard_page();
        } else {
            display_mnemonic_page();
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
    PRINTF("Current text is: '%s' (size '%d')\n", textToEnter, textLen);
    if (textLen < 2) {
        // no suggestion until there is at least 2 characters
        nbgl_layoutUpdateSuggestionButtons(layout, suggestionIndex, 0, buttonTexts);
    } else {
        const size_t nbMatchingWords =
            bolos_ux_bip39_fill_with_candidates((unsigned char *) &(textToEnter[0]),
                                                strlen(textToEnter),
                                                wordCandidates,
                                                buttonTexts);
        nbgl_layoutUpdateSuggestionButtons(layout, suggestionIndex, nbMatchingWords, buttonTexts);
    }
    if (textLen > 0) {
        mask = bolos_ux_bip39_get_keyboard_mask((unsigned char *) &(textToEnter[0]),
                                                strlen(textToEnter));
    }
    nbgl_layoutUpdateKeyboard(layout, keyboardIndex, mask);
    nbgl_layoutUpdateEnteredText(layout, textIndex, false, 0, &(textToEnter[0]), false);
    nbgl_refresh();
}

static void display_keyboard_page() {
    nbgl_layoutDescription_t layoutDescription = {.modal = false,
                                                  .onActionCallback = &keyboard_dispatcher};
    nbgl_layoutKbd_t kbdInfo = {.lettersOnly = true,   // use only letters
                                .upperCase = false,    // start with lower case letters
                                .mode = MODE_LETTERS,  // start in letters mode
                                .keyMask = 0,          // no inactive key
                                .callback = &key_press_callback};
    nbgl_layoutCenteredInfo_t centeredInfo = {.text1 = NULL,
                                              .text2 = headerText,  // to use as "header"
                                              .text3 = NULL,
                                              .style = LARGE_CASE_INFO,
                                              .icon = NULL,
                                              .offsetY = 0,
                                              .onTop = true};
    strlcpy(textToEnter, "", 1);
    memset(buttonTexts, 0, NB_MAX_SUGGESTION_BUTTONS);
    layout = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddProgressIndicator(layout, 0, 0, true, BACK_BUTTON_TOKEN, TUNE_TAP_CASUAL);
    memset(headerText, 0, HEADER_SIZE);
    snprintf(headerText,
             HEADER_SIZE,
             "Enter word n. %d/%d from your\nrecovery passphrase",
             get_current_word_number() + 1,
             get_mnemonic_final_size());
    nbgl_layoutAddCenteredInfo(layout, &centeredInfo);
    keyboardIndex = nbgl_layoutAddKeyboard(layout, &kbdInfo);
    textIndex = nbgl_layoutAddEnteredText(layout,
                                          true,                           // numbered
                                          get_current_word_number() + 1,  // number to use
                                          textToEnter,                    // text to display
                                          false,                          // not grayed-out
                                          32);  // vertical margin from the buttons
    suggestionIndex = nbgl_layoutAddSuggestionButtons(layout,
                                                      0,  // no used buttons at start-up
                                                      buttonTexts,
                                                      FIRST_SUGGESTION_TOKEN,
                                                      TUNE_TAP_CASUAL);
    nbgl_layoutDraw(layout);
}

/*
 * Home page & dispatcher
 */

static void display_settings_page() {
    nbgl_useCaseSettings("Recovery Check infos", 0, 1, false, onHome, onInfos, NULL);
}

static void display_home_page() {
    reset_globals();
    nbgl_useCaseHomeExt("Recovery Check",
                        &C_fatstacks_recovery_check_64px,
                        "Verify the validity of\nyour recovery passphrase",
                        true,
                        "Check your passphrase",
                        display_mnemonic_page,
                        display_settings_page,
                        onQuit);
}

/*
 * Result page
 */
static char *possible_results[2] = {"Sorry, this recovery\npassphrase is\nincorrect.",
                                    "Your recovery\npassphrase is\ncorrect!"};

static void display_result_page(const bool result) {
    reset_globals();
    nbgl_useCaseHomeExt(possible_results[result],
                        &C_fatstacks_recovery_check_64px,
                        "",
                        false,
                        "Check another passphrase",
                        display_mnemonic_page,
                        NULL,
                        onQuit);
}

/*
 * Public function
 */
void ui_idle_init(void) {
    display_home_page();
}

#endif
