#include <ux.h>
#include "ui.h"

#if defined(HAVE_NBGL)

#include <nbgl_fonts.h>
#include <nbgl_front.h>
#include <nbgl_debug.h>
#include <nbgl_page.h>

#include "ux_common/common_bip39.h"

#define HEADER_SIZE 50

#define MAX_MNEMONIC_LENGTH (MNEMONIC_SIZE_24 * (MAX_WORD_LENGTH+1))

static nbgl_page_t* pageContext;
static char headerText[HEADER_SIZE] = {0};
static nbgl_layout_t *layout = 0;

void ui_menu_about(void);
void display_keyboard_page(void);
void display_home_page(void);
void display_result_page(const bool result);
void display_mnemonic_page(void);
void reset_globals(void);

void releaseContext(void) {
    if (pageContext != NULL) {
        nbgl_pageRelease(pageContext);
        pageContext = NULL;
    }
}

enum {
    BACK_HOME_TOKEN = 0,
    BACK_BUTTON_TOKEN,
    CHOOSE_MNEMONIC_SIZE_TOKEN,
    FIRST_SUGGESTION_TOKEN,
    INFO_TOKEN,
    QUIT_APP_TOKEN,
    START_RECOVER_TOKEN,
};

void pageTouchCallback(int token, uint8_t index __attribute__((unused))) {
    if (token == QUIT_APP_TOKEN) {
        releaseContext();
        os_sched_exit(-1);
    } else if (token == INFO_TOKEN) {
        ui_menu_about();
    } else if (token == CHOOSE_MNEMONIC_SIZE_TOKEN) {
        display_mnemonic_page();
    } else if (token == BACK_HOME_TOKEN) {
        releaseContext();
        ui_idle_init();
    }
}

/*
 * About menu
 */
static const char* const infoTypes[] = {"Version", "Recovery Check"};
static const char* const infoContents[] = {APPVERSION, "(c) 2022 Ledger"};

void ui_menu_about() {
    nbgl_pageContent_t content = {.title = "Recovery Check infos", .isTouchableTitle = false};
    nbgl_pageNavigationInfo_t nav = {.activePage = 0,
                                     .nbPages = 1,
                                     .navType = NAV_WITH_BUTTONS,
                                     .navWithButtons.quitButton = true,
                                     .navWithButtons.navToken = BACK_HOME_TOKEN,
                                     .tuneId = TUNE_TAP_CASUAL};
    content.type = INFOS_LIST;
    content.infosList.nbInfos = 2;
    content.infosList.infoTypes = (const char**) infoTypes;
    content.infosList.infoContents = (const char**) infoContents;

    releaseContext();
    pageContext = nbgl_pageDrawGenericContent(&pageTouchCallback, &nav, &content);
    nbgl_refresh();
}

/*
 * Choose mnemonic size page
 */
void mnemonic_dispatcher(const int token, uint8_t index) {
    if (token == BACK_BUTTON_TOKEN) {
        nbgl_layoutRelease(layout);
        display_home_page();
    } else if (token == CHOOSE_MNEMONIC_SIZE_TOKEN) {
        switch(index) {
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

void display_mnemonic_page() {
    reset_globals();
    nbgl_layoutDescription_t layoutDescription = {
      .modal = false,
      .onActionCallback = mnemonic_dispatcher
    };
    nbgl_layoutRadioChoice_t choices = {
        .names = (char*[]){"12 words", "18 words", "24 words"},
        .localized = false,
        .nbChoices = 3,
        .initChoice = 2,
        .token = CHOOSE_MNEMONIC_SIZE_TOKEN
    };
    nbgl_layoutCenteredInfo_t centeredInfo = {
      .text1 = NULL,
      .text2 = headerText, // to use as "header"
      .text3 = NULL,
      .style = LARGE_CASE_INFO,
      .icon = NULL,
      .offsetY = 0,
      .onTop = true
    };
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
static char *buttonTexts[NB_MAX_SUGGESTION_BUTTONS] = {0};
// the biggest word of BIP39 list is 8 char (9 with trailing '\0'), and
// the max number of showed suggestions is NB_MAX_SUGGESTION_BUTTONS
static char wordCandidates[(MAX_WORD_LENGTH + 1) * NB_MAX_SUGGESTION_BUTTONS] = {0};

// function called when back or any suggestion button is touched
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
        add_word_in_mnemonic(
            buttonTexts[token - FIRST_SUGGESTION_TOKEN],
            strlen(buttonTexts[token - FIRST_SUGGESTION_TOKEN])
            );
        // current_word starts at 1
        if (is_mnemonic_complete()) {
            display_result_page(check_mnemonic());
        } else {
            display_keyboard_page();
        }
        // go back to main screen of app
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
        const size_t nbMatchingWords = bolos_ux_bip39_fill_with_candidates(
            (unsigned char *)&(textToEnter[0]),
            strlen(textToEnter),
            wordCandidates,
            buttonTexts
        );
        nbgl_layoutUpdateSuggestionButtons(layout, suggestionIndex, nbMatchingWords, buttonTexts);
    }
    if (textLen > 0) {
        mask = bolos_ux_bip39_get_keyboard_mask(
            (unsigned char *)&(textToEnter[0]),
            strlen(textToEnter)
            );
    }
    nbgl_layoutUpdateKeyboard(layout, keyboardIndex, mask);
    nbgl_layoutUpdateEnteredText(layout, textIndex, false, 0, &(textToEnter[0]), false);
    nbgl_refresh();
 }

void display_keyboard_page() {
    nbgl_layoutDescription_t layoutDescription = {
      .modal = false,
      .onActionCallback = &keyboard_dispatcher
    };
    nbgl_layoutKbd_t kbdInfo = {
      .lettersOnly = true,  // use only letters
      .upperCase = false,   // start with lower case letters
      .mode = MODE_LETTERS, // start in letters mode
      .keyMask = 0,         // no inactive key
      .callback = &key_press_callback
    };
    nbgl_layoutCenteredInfo_t centeredInfo = {
      .text1 = NULL,
      .text2 = headerText, // to use as "header"
      .text3 = NULL,
      .style = LARGE_CASE_INFO,
      .icon = NULL,
      .offsetY = 0,
      .onTop = true
    };
    strlcpy(textToEnter, "", 1);
    memset(buttonTexts, 0, NB_MAX_SUGGESTION_BUTTONS);

    layout = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddProgressIndicator(layout, 0, 0, true, BACK_BUTTON_TOKEN, TUNE_TAP_CASUAL);
    memset(headerText, 0, HEADER_SIZE);
    snprintf(
        headerText,
        HEADER_SIZE,
        "Enter word n. %d/%d from your\nRecovery Sheet",
        get_current_word_number() + 1,
        get_mnemonic_final_size()
        );
    nbgl_layoutAddCenteredInfo(layout, &centeredInfo);
    keyboardIndex = nbgl_layoutAddKeyboard(layout, &kbdInfo);
    textIndex = nbgl_layoutAddEnteredText(layout,
                                          true,         // numbered
                                          get_current_word_number() + 1, // number to use
                                          textToEnter,  // text to display
                                          false,        // not grayed-out
                                          32);          // vertical margin from the buttons
    suggestionIndex = nbgl_layoutAddSuggestionButtons(layout,
                                                      0, // no used buttons at start-up
                                                      buttonTexts,
                                                      FIRST_SUGGESTION_TOKEN,
                                                      TUNE_TAP_CASUAL);
    nbgl_layoutDraw(layout);
}

/*
 * Home page
 */

static void display_home_page() {
    nbgl_pageInfoDescription_t home = {
        /* .centeredInfo.icon = &C_fatstacks_app_recovery_check, */
        .centeredInfo.icon = NULL,
        .centeredInfo.text1 = "Recovery Check app",
        .centeredInfo.text2 = NULL,
        .centeredInfo.text3 = NULL,
        .centeredInfo.style = LARGE_CASE_INFO,
        .centeredInfo.offsetY = 32,
        .topRightStyle = INFO_ICON,
        .bottomButtonStyle = QUIT_ICON,
        .topRightToken = INFO_TOKEN,
        .bottomButtonToken = QUIT_APP_TOKEN,
        .footerText = NULL,
        .tapActionText = "Tap to check your mnemonic",
        .tapActionToken = CHOOSE_MNEMONIC_SIZE_TOKEN,
        .tuneId = TUNE_TAP_CASUAL
    };
    releaseContext();
    pageContext = nbgl_pageDrawInfo(&pageTouchCallback, NULL, &home);
    nbgl_refresh();
}

/*
 * Result page
 */
static char *possible_results[2] = {
    "Sorry, this passphrase\nis incorrect.",
    "You passphrase\nis correct!"
};

static void display_result_page(const bool result) {
    nbgl_pageInfoDescription_t home = {
        /* .centeredInfo.icon = &C_fatstacks_app_recovery_check, */
        .centeredInfo.icon = NULL,
        .centeredInfo.text1 = possible_results[result],
        .centeredInfo.text2 = NULL,
        .centeredInfo.text3 = NULL,
        .centeredInfo.style = LARGE_CASE_INFO,
        .centeredInfo.offsetY = 32,
        .topRightStyle = NO_BUTTON_STYLE,
        .bottomButtonStyle = QUIT_ICON,
        .bottomButtonToken = QUIT_APP_TOKEN,
        .footerText = NULL,
        .tapActionText = "Tap to check another mnemonic",
        .tapActionToken = CHOOSE_MNEMONIC_SIZE_TOKEN,
        .tuneId = TUNE_TAP_CASUAL
    };
    releaseContext();
    pageContext = nbgl_pageDrawInfo(&pageTouchCallback, NULL, &home);
    nbgl_refresh();
}

/*
 * Utils
 */

static void reset_globals() {
    reset_mnemonic();
    memset(buttonTexts, 0, NB_MAX_SUGGESTION_BUTTONS);
}

#endif


enum UI_STATE { UI_IDLE, UI_TEXT, UI_APPROVAL };

enum UI_STATE uiState;

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

#if defined(TARGET_NANOS)

UX_STEP_CB(restore_3_1_1, bb, G_bolos_ux_context.onboarding_kind = MNEMONIC_SIZE_24;
           screen_onboarding_4_restore_word_init(RESTORE_WORD_ACTION_FIRST_WORD);
           ,
           {
               "Recovery phrase",
               "with 24 words",
           });

UX_STEP_CB(restore_3_1_2, bb, G_bolos_ux_context.onboarding_kind = MNEMONIC_SIZE_18;
           screen_onboarding_4_restore_word_init(RESTORE_WORD_ACTION_FIRST_WORD);
           ,
           {
               "Recovery phrase",
               "with 18 words",
           });

UX_STEP_CB(restore_3_1_3, bb, G_bolos_ux_context.onboarding_kind = MNEMONIC_SIZE_12;
           screen_onboarding_4_restore_word_init(RESTORE_WORD_ACTION_FIRST_WORD);
           ,
           {
               "Recovery phrase",
               "with 12 words",
           });

UX_FLOW(restore_3_1, &restore_3_1_1, &restore_3_1_2, &restore_3_1_3);

void screen_onboarding_3_restore_init(void) {
    ux_flow_init(0, restore_3_1, NULL);
}

UX_STEP_VALID(ux_idle_flow_1_step, pbb, screen_onboarding_3_restore_init();,
                                                                           {
                                                                               &C_badge,
                                                                               "Check your",
                                                                               "recovery phrase",
                                                                           });

UX_STEP_NOCB(ux_idle_flow_3_step,
             bn,
             {
                 "Version",
                 APPVERSION,
             });

UX_STEP_VALID(ux_idle_flow_4_step,
              pb,
              os_sched_exit(-1),
              {
                  &C_icon_dashboard_x,
                  "Quit",
              });

UX_FLOW(ux_idle_flow, &ux_idle_flow_1_step, &ux_idle_flow_3_step, &ux_idle_flow_4_step);

#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)

//////////////////////////////////////////////////////////////////////

const char* const number_of_words_getter_values[] = {
    "12 words",
    "18 words",
    "24 words",
    "Back",
};

const char* number_of_words_getter(unsigned int idx) {
    if (idx < ARRAYLEN(number_of_words_getter_values)) {
        return number_of_words_getter_values[idx];
    }
    return NULL;
}

void number_of_words_selector(unsigned int idx) {
    switch (idx) {
        case 0:
            G_bolos_ux_context.onboarding_kind = MNEMONIC_SIZE_12;
            screen_onboarding_4_restore_word_init(1 /*entering the first word*/);
            break;
        case 1:
            G_bolos_ux_context.onboarding_kind = MNEMONIC_SIZE_18;
            screen_onboarding_4_restore_word_init(1 /*entering the first word*/);
            break;
        case 2:
            G_bolos_ux_context.onboarding_kind = MNEMONIC_SIZE_24;
            screen_onboarding_4_restore_word_init(1 /*entering the first word*/);
            break;
        default:
            ui_idle_init();
    }
}

//////////////////////////////////////////////////////////////////////

UX_STEP_VALID(ux_instruction_step,
              nnn,
              ux_menulist_init(0, number_of_words_getter, number_of_words_selector),
              {
                  "Select the number",
                  "of words written on",
                  "your Recovery Sheet",
              });

UX_FLOW(ux_instruction_flow, &ux_instruction_step);

//////////////////////////////////////////////////////////////////////

UX_STEP_VALID(ux_idle_flow_1_step,
              pbb,
              ux_flow_init(0, ux_instruction_flow, NULL),
              {
                  &C_badge,
                  "Check your",
                  "recovery phrase",
              });
UX_STEP_NOCB(ux_idle_flow_3_step,
             bn,
             {
                 "Version",
                 APPVERSION,
             });
UX_STEP_VALID(ux_idle_flow_4_step,
              pb,
              os_sched_exit(-1),
              {
                  &C_icon_dashboard_x,
                  "Quit",
              });
UX_FLOW(ux_idle_flow, &ux_idle_flow_1_step, &ux_idle_flow_3_step, &ux_idle_flow_4_step);

#endif


void ui_idle_init(void) {
#if defined(HAVE_BAGL)
    uiState = UI_IDLE;

    // reserve a display stack slot if none yet
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_idle_flow, NULL);
#endif

#if defined(HAVE_NBGL)
    reset_globals();
    display_home_page();
#endif

}
