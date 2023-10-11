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
#include "../ui.h"
#include "./mnemonic.h"
#include "./layout_generic_screen.h"

#define HEADER_SIZE 50

static nbgl_page_t *pageContext;

static char headerText[HEADER_SIZE] = {0};
static nbgl_layout_t *layout = 0;

static void display_home_page(void);
static void display_select_bip39_passphrase_length_page(void);
static void display_bip39_keyboard_page(void);
static void display_result_page(const bool result);
static void display_numshares_keypad_page(void);

enum bip39_check {
    BACK_BUTTON_TOKEN = FIRST_USER_TOKEN,
    SELECT_MNEMONIC_SIZE_TOKEN,
    FIRST_SUGGESTION_TOKEN,
    START_RECOVER_TOKEN,
    RESULT_TOKEN,
};

/*
 * Utils
 */
static const char *buttonTexts[NB_MAX_SUGGESTION_BUTTONS] = {0};

static void reset_globals() {
    reset_mnemonic();
    memzero(buttonTexts, sizeof(buttonTexts[0]) * NB_MAX_SUGGESTION_BUTTONS);
}

static void on_quit(void) {
    os_sched_exit(-1);
}

/*
 * About menu
 */
static const char *const infoTypes[] = {"Version", APPNAME};
static const char *const infoContents[] = {APPVERSION, "(c) 2023 Ledger"};

static bool on_infos(uint8_t page, nbgl_pageContent_t *content) {
    if (page == 0) {
        content->type = INFOS_LIST;
        content->infosList.nbInfos = 2;
        content->infosList.infoTypes = infoTypes;
        content->infosList.infoContents = infoContents;
        return true;
    }
    return false;
}

/*
 * Select recovery type, BIP39 or SSKR
 */
enum select_check_type {
    SELECT_CHECK_TYPE_ICON_INDEX = 0,
    SELECT_CHECK_TYPE_TEXT_INDEX,
    SELECT_CHECK_TYPE_BIP39_INDEX,
    SELECT_CHECK_TYPE_SSKR_INDEX,
    SELECT_CHECK_TYPE_BACK_BUTTON_INDEX,
    SELECT_CHECK_TYPE_KBD_TEXT_TOKEN,
    SELECT_CHECK_TYPE_NB_CHILDREN
};

#define SELECT_CHECK_TYPE_NB_BUTTONS 3

static const char *checkType[] = {"BIP39 Check", "SSKR Check"};
static void select_check_type_callback(nbgl_obj_t *obj, nbgl_touchType_t eventType) {
    nbgl_obj_t **screenChildren = nbgl_screenGetElements(0);
    if (eventType != TOUCHED) {
        return;
    }
    io_seproxyhal_play_tune(TUNE_TAP_CASUAL);
    if (obj == screenChildren[SELECT_CHECK_TYPE_BIP39_INDEX]) {
        nbgl_layoutRelease(layout);
        display_select_bip39_passphrase_length_page();
    } else if (obj == screenChildren[SELECT_CHECK_TYPE_SSKR_INDEX]) {
        nbgl_layoutRelease(layout);
        display_home_page();
        return;
    } else if (obj == screenChildren[SELECT_CHECK_TYPE_BACK_BUTTON_INDEX]) {
        nbgl_layoutRelease(layout);
        display_home_page();
        return;
    }
}

static void display_select_check_type_page(void) {
    nbgl_obj_t **screenChildren;

    // 2 buttons + icon + text + subText
    nbgl_screenSet(&screenChildren, 5, NULL, (nbgl_touchCallback_t) &select_check_type_callback);

    screenChildren[SELECT_CHECK_TYPE_ICON_INDEX] =
        (nbgl_obj_t *) generic_screen_set_icon(&C_seed_stax_64px);
    screenChildren[SELECT_CHECK_TYPE_TEXT_INDEX] =
        (nbgl_obj_t *) generic_screen_set_title(screenChildren[SELECT_CHECK_TYPE_ICON_INDEX]);
    ((nbgl_text_area_t *) screenChildren[SELECT_CHECK_TYPE_TEXT_INDEX])->text =
        "Select type of\nRecovery you wish\nto check";
    // create nb words buttons
    nbgl_objPoolGetArray(BUTTON,
                         SELECT_CHECK_TYPE_NB_BUTTONS,
                         0,
                         (nbgl_obj_t **) &screenChildren[SELECT_CHECK_TYPE_BIP39_INDEX]);
    generic_screen_configure_buttons(
        (nbgl_button_t **) &screenChildren[SELECT_CHECK_TYPE_BIP39_INDEX],
        SELECT_CHECK_TYPE_NB_BUTTONS);
    ((nbgl_button_t *) screenChildren[SELECT_CHECK_TYPE_BIP39_INDEX])->text = checkType[0];
    ((nbgl_button_t *) screenChildren[SELECT_CHECK_TYPE_BIP39_INDEX])->icon = &C_bip39_stax_32px;
    ((nbgl_button_t *) screenChildren[SELECT_CHECK_TYPE_SSKR_INDEX])->text = checkType[1];
    ((nbgl_button_t *) screenChildren[SELECT_CHECK_TYPE_SSKR_INDEX])->icon = &C_sskr_stax_32px;
    ((nbgl_button_t *) screenChildren[SELECT_CHECK_TYPE_SSKR_INDEX])->borderColor = BLACK;
    ((nbgl_button_t *) screenChildren[SELECT_CHECK_TYPE_SSKR_INDEX])->innerColor = BLACK;
    ((nbgl_button_t *) screenChildren[SELECT_CHECK_TYPE_SSKR_INDEX])->foregroundColor = WHITE;

    // create back button
    screenChildren[SELECT_CHECK_TYPE_BACK_BUTTON_INDEX] =
        (nbgl_obj_t *) generic_screen_set_back_button();

    nbgl_screenRedraw();
}

/*
 * Select Generate SSKR or BIP85
 */
enum select_generate_type {
    SELECT_GENERATE_TYPE_ICON_INDEX = 0,
    SELECT_GENERATE_TYPE_TEXT_INDEX,
    SELECT_GENERATE_TYPE_BIP85_INDEX,
    SELECT_GENERATE_TYPE_SSKR_INDEX,
    SELECT_GENERATE_TYPE_BACK_BUTTON_INDEX,
    SELECT_GENERATE_TYPE_KBD_TEXT_TOKEN,
    SELECT_GENERATE_TYPE_NB_CHILDREN
};

#define SELECT_GENERATE_TYPE_NB_BUTTONS 2

static const char *generateType[] = {"Generate BIP85", "Generate SSKR"};
static void select_generate_type_callback(nbgl_obj_t *obj, nbgl_touchType_t eventType) {
    nbgl_obj_t **screenChildren = nbgl_screenGetElements(0);
    if (eventType != TOUCHED) {
        return;
    }
    io_seproxyhal_play_tune(TUNE_TAP_CASUAL);
    if (obj == screenChildren[SELECT_GENERATE_TYPE_BIP85_INDEX]) {
        nbgl_layoutRelease(layout);
        display_home_page();
    } else if (obj == screenChildren[SELECT_GENERATE_TYPE_SSKR_INDEX]) {
        nbgl_layoutRelease(layout);
        display_numshares_keypad_page();
    } else if (obj == screenChildren[SELECT_GENERATE_TYPE_BACK_BUTTON_INDEX]) {
        nbgl_layoutRelease(layout);
        display_home_page();
        return;
    }
}

static void display_select_generate_type_page(void) {
    nbgl_obj_t **screenChildren;

    // 2 buttons + icon + text + subText
    nbgl_screenSet(&screenChildren, 5, NULL, (nbgl_touchCallback_t) &select_generate_type_callback);

    screenChildren[SELECT_GENERATE_TYPE_ICON_INDEX] =
        (nbgl_obj_t *) generic_screen_set_icon(&C_seed_stax_64px);
    screenChildren[SELECT_GENERATE_TYPE_TEXT_INDEX] =
        (nbgl_obj_t *) generic_screen_set_title(screenChildren[SELECT_GENERATE_TYPE_ICON_INDEX]);
    ((nbgl_text_area_t *) screenChildren[SELECT_GENERATE_TYPE_TEXT_INDEX])->text =
        "Select if you wish to\ngenerate SSKR shares\nor BIP85 children";
    // create nb words buttons
    nbgl_objPoolGetArray(BUTTON,
                         SELECT_GENERATE_TYPE_NB_BUTTONS,
                         0,
                         (nbgl_obj_t **) &screenChildren[SELECT_GENERATE_TYPE_BIP85_INDEX]);
    generic_screen_configure_buttons(
        (nbgl_button_t **) &screenChildren[SELECT_GENERATE_TYPE_BIP85_INDEX],
        SELECT_GENERATE_TYPE_NB_BUTTONS);
    ((nbgl_button_t *) screenChildren[SELECT_GENERATE_TYPE_BIP85_INDEX])->text = generateType[0];
    ((nbgl_button_t *) screenChildren[SELECT_GENERATE_TYPE_BIP85_INDEX])->icon = &C_bip85_stax_32px;
    ((nbgl_button_t *) screenChildren[SELECT_GENERATE_TYPE_SSKR_INDEX])->text = generateType[1];
    ((nbgl_button_t *) screenChildren[SELECT_GENERATE_TYPE_SSKR_INDEX])->icon = &C_sskr_stax_32px;
    ((nbgl_button_t *) screenChildren[SELECT_GENERATE_TYPE_SSKR_INDEX])->borderColor = BLACK;
    ((nbgl_button_t *) screenChildren[SELECT_GENERATE_TYPE_SSKR_INDEX])->innerColor = BLACK;
    ((nbgl_button_t *) screenChildren[SELECT_GENERATE_TYPE_SSKR_INDEX])->foregroundColor = WHITE;

    // create back button
    screenChildren[SELECT_GENERATE_TYPE_BACK_BUTTON_INDEX] =
        (nbgl_obj_t *) generic_screen_set_back_button();

    nbgl_screenRedraw();
}

/*
 * Select mnemonic size page
 */
enum select_bip_passphrase_length {
    SELECT_BIP39_PASSPHRASE_LENGTH_ICON_INDEX = 0,
    SELECT_BIP39_PASSPHRASE_LENGTH_TEXT_INDEX,
    SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_12_INDEX,
    SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_18_INDEX,
    SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_24_INDEX,
    SELECT_BIP39_PASSPHRASE_LENGTH_BACK_BUTTON_INDEX,
    SELECT_BIP39_PASSPHRASE_LENGTH_KBD_TEXT_TOKEN,
    SELECT_BIP39_PASSPHRASE_LENGTH_NB_CHILDREN
};

#define SELECT_BIP39_PASSPHRASE_LENGTH_NB_BUTTONS 3

static const char *bip39_passphraseLength[] = {"12 words", "18 words", "24 words"};
static void select_bip39_passphrase_length_callback(nbgl_obj_t *obj, nbgl_touchType_t eventType) {
    nbgl_obj_t **screenChildren = nbgl_screenGetElements(0);
    if (eventType != TOUCHED) {
        return;
    }
    io_seproxyhal_play_tune(TUNE_TAP_CASUAL);
    if (obj == screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_12_INDEX]) {
        set_mnemonic_final_size(MNEMONIC_SIZE_12);
    } else if (obj == screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_18_INDEX]) {
        set_mnemonic_final_size(MNEMONIC_SIZE_18);
    } else if (obj == screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_24_INDEX]) {
        set_mnemonic_final_size(MNEMONIC_SIZE_24);
    } else if (obj == screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BACK_BUTTON_INDEX]) {
        nbgl_layoutRelease(layout);
        display_select_check_type_page();
        return;
    }
    nbgl_layoutRelease(layout);
    display_bip39_keyboard_page();
}

static void display_select_bip39_passphrase_length_page(void) {
    nbgl_obj_t **screenChildren;

    // 3 buttons + icon + text + subText
    nbgl_screenSet(&screenChildren,
                   6,
                   NULL,
                   (nbgl_touchCallback_t) &select_bip39_passphrase_length_callback);

    screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_ICON_INDEX] =
        (nbgl_obj_t *) generic_screen_set_icon(&C_bip39_stax_64px);
    screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_TEXT_INDEX] =
        (nbgl_obj_t *) generic_screen_set_title(
            screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_ICON_INDEX]);
    ((nbgl_text_area_t *) screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_TEXT_INDEX])->text =
        "How long is your\nBIP39 Recovery\nPhrase?";

    // create nb words buttons
    nbgl_objPoolGetArray(
        BUTTON,
        SELECT_BIP39_PASSPHRASE_LENGTH_NB_BUTTONS,
        0,
        (nbgl_obj_t **) &screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_12_INDEX]);
    generic_screen_configure_buttons(
        (nbgl_button_t **) &screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_12_INDEX],
        SELECT_BIP39_PASSPHRASE_LENGTH_NB_BUTTONS);
    ((nbgl_button_t *) screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_12_INDEX])->text =
        bip39_passphraseLength[0];
    ((nbgl_button_t *) screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_18_INDEX])->text =
        bip39_passphraseLength[1];
    ((nbgl_button_t *) screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_24_INDEX])->text =
        bip39_passphraseLength[2];
    ((nbgl_button_t *) screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_24_INDEX])
        ->borderColor = BLACK;
    ((nbgl_button_t *) screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_24_INDEX])->innerColor =
        BLACK;
    ((nbgl_button_t *) screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BUTTON_24_INDEX])
        ->foregroundColor = WHITE;

    // create back button
    screenChildren[SELECT_BIP39_PASSPHRASE_LENGTH_BACK_BUTTON_INDEX] =
        (nbgl_obj_t *) generic_screen_set_back_button();

    nbgl_screenRedraw();
}

/*
 * Word recover page
 */
#define BUTTON_VMARGIN 32

static char textToEnter[MAX_WORD_LENGTH + 1] = {0};
static int textIndex, suggestionIndex, keyboardIndex = 0;
// the biggest word of BIP39 list is 8 char (9 with trailing '\0'), and
// the max number of showed suggestions is NB_MAX_SUGGESTION_BUTTONS
static char wordCandidates[(MAX_WORD_LENGTH + 1) * NB_MAX_SUGGESTION_BUTTONS] = {0};

static void keyboard_dispatcher(const int token, uint8_t index __attribute__((unused))) {
    if (token == BACK_BUTTON_TOKEN) {
        nbgl_layoutRelease(layout);
        if (remove_word_from_mnemonic()) {
            display_bip39_keyboard_page();
        } else {
            display_select_bip39_passphrase_length_page();
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
            display_bip39_keyboard_page();
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
    if (textLen == 0) {
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
    nbgl_layoutUpdateKeyboard(layout, keyboardIndex, mask, false, LOWER_CASE);
    nbgl_layoutUpdateEnteredText(layout, textIndex, false, 0, &(textToEnter[0]), false);
    nbgl_refresh();
}

static void display_bip39_keyboard_page() {
    nbgl_layoutDescription_t layoutDescription = {.modal = false,
                                                  .onActionCallback = &keyboard_dispatcher};
    nbgl_layoutKbd_t kbdInfo = {.lettersOnly = true,   // use only letters
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
    textToEnter[0] = '\0';
    memzero(buttonTexts, sizeof(buttonTexts[0]) * NB_MAX_SUGGESTION_BUTTONS);
    layout = nbgl_layoutGet(&layoutDescription);
    snprintf(headerText,
             HEADER_SIZE,
             "Enter word n. %d/%d of your\nBIP39 Recovery Phrase",
             get_current_word_number() + 1,
             get_mnemonic_final_size());
    nbgl_layoutAddProgressIndicator(layout, 0, 0, true, BACK_BUTTON_TOKEN, TUNE_TAP_CASUAL);
    nbgl_layoutAddCenteredInfo(layout, &centeredInfo);
    keyboardIndex = nbgl_layoutAddKeyboard(layout, &kbdInfo);
    suggestionIndex = nbgl_layoutAddSuggestionButtons(layout,
                                                      0,  // no used buttons at start-up
                                                      buttonTexts,
                                                      FIRST_SUGGESTION_TOKEN,
                                                      TUNE_TAP_CASUAL);
    textIndex = nbgl_layoutAddEnteredText(layout,
                                          true,                           // numbered
                                          get_current_word_number() + 1,  // number to use
                                          textToEnter,                    // text to display
                                          false,                          // not grayed-out
                                          BUTTON_VMARGIN,  // vertical margin from the buttons
                                          SELECT_BIP39_PASSPHRASE_LENGTH_KBD_TEXT_TOKEN);
    nbgl_layoutDraw(layout);
}

/*
 * Home page & dispatcher
 */

static void display_settings_page() {
    char infos[] = " infos";
    size_t len = sizeof(APPNAME) + sizeof(infos);
    char appname_infos[len];

    strlcpy(appname_infos, APPNAME, len);
    strlcat(appname_infos, infos, len);
    nbgl_useCaseSettings(appname_infos, 0, 1, false, display_home_page, on_infos, NULL);
}

static void display_home_page() {
    reset_globals();
    nbgl_useCaseHomeExt(APPNAME,
                        &C_seed_stax_64px,
                        "This app lets you enter a\nSecret Recovery Phrase and\ntest if it matches "
                        "the one\npresent on this Ledger Stax",
                        false,
                        "Recovery Check",
                        display_select_check_type_page,
                        display_settings_page,
                        on_quit);
}

/*
 * Result page
 */
static const char *possible_results[2][2] = {
    {"Incorrect Secret\nRecovery Phrase",
     "The Recovery Phrase you have\nentered doesn't match the one\npresent on this Ledger Stax."},
    {"Correct Secret\nRecovery Phrase",
     "The Recovery Phrase you have\nentered matches the one\npresent on this Ledger Stax."}};
static const nbgl_icon_details_t *icons[2] = {&C_warning64px, &C_round_check_64px};

static void result_callback(int token __attribute__((unused)),
                            uint8_t index __attribute__((unused))) {
    if (is_mnemonic_complete()) {
        display_select_generate_type_page();
    } else {
        reset_globals();
        display_home_page();
    }
}

static void display_result_page(const bool result) {
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
 * Select number of shares page
 */

enum sskr_gen {
    SSKR_GEN_BACK_BUTTON_TOKEN = FIRST_USER_TOKEN,
    SSKR_GEN_SELECT_SHARENUM_TOKEN,
    SSKR_GEN_SELECT_THRESHOLD_TOKEN,
    SSKR_GEN_RESULT_TOKEN,
};

static char numToEnter[MAX_NUMBER_LENGTH + 1] = {0};
static int keypadIndex = 0;

static void keypad_dispatcher(const int token, uint8_t index __attribute__((unused))) {
    if (token == SSKR_GEN_BACK_BUTTON_TOKEN) {
        nbgl_layoutRelease(layout);
        display_select_generate_type_page();
    } else if (token >= FIRST_SUGGESTION_TOKEN) {
        nbgl_layoutRelease(layout);
        PRINTF("Selected number is '%d'\n", buttonTexts[token - FIRST_SUGGESTION_TOKEN]);
    }
}

// function called when a key of keypad is touched
static void keypad_press_callback(const char touchedKey __attribute__((unused))) {
    // Function currently a placeholder, remove unused attribute tag
}

static void display_numshares_keypad_page() {
    nbgl_layoutDescription_t layoutDescription = {.modal = false,
                                                  .onActionCallback = &keypad_dispatcher};
    nbgl_layoutCenteredInfo_t centeredInfo = {.text1 = NULL,
                                              .text2 = headerText,  // to use as "header"
                                              .text3 = NULL,
                                              .style = LARGE_CASE_INFO,
                                              .icon = NULL,
                                              .offsetY = 0,
                                              .onTop = true};
    numToEnter[0] = '\0';
    layout = nbgl_layoutGet(&layoutDescription);
    snprintf(headerText, HEADER_SIZE, "Enter number of SSKR shares\nto generate (1 - 16)");
    nbgl_layoutAddProgressIndicator(layout,
                                    0,
                                    0,
                                    true,
                                    SSKR_GEN_BACK_BUTTON_TOKEN,
                                    TUNE_TAP_CASUAL);
    nbgl_layoutAddCenteredInfo(layout, &centeredInfo);
    keypadIndex = nbgl_layoutAddKeypad(layout, &keypad_press_callback, false);
    textIndex = nbgl_layoutAddEnteredText(layout,
                                          false,           // numbered
                                          0,               // number to use
                                          numToEnter,      // num to display
                                          false,           // not grayed-out
                                          BUTTON_VMARGIN,  // vertical margin from the buttons
                                          SSKR_GEN_SELECT_THRESHOLD_TOKEN);
    nbgl_layoutDraw(layout);
}

/*
 * Public function
 */
void ui_idle_init(void) {
    display_home_page();
}

#endif
