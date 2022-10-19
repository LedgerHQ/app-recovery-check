#include <ux.h>
#include "ui.h"

#if defined(HAVE_NBGL)

#include <nbgl_fonts.h>
#include <nbgl_front.h>
#include <nbgl_debug.h>
#include <nbgl_page.h>

#include "ux_common/common_bip39.h"

static nbgl_page_t* pageContext;
static int wordNum;

void ui_menu_about(void);
void page_keyboard(void);
void display_home_page(void);

void releaseContext(void) {
    if (pageContext != NULL) {
        nbgl_pageRelease(pageContext);
        pageContext = NULL;
    }
}

enum {
    BACK_HOME_TOKEN = 0,
    BACK_BUTTON_TOKEN,
    FIRST_SUGGESTION_TOKEN,
    INFO_TOKEN,
    QUIT_APP_TOKEN,
    START_RECOVER_TOKEN,
};

void pageTouchCallback(int token, uint8_t index) {
    (void) index;
    if (token == QUIT_APP_TOKEN) {
        releaseContext();
        os_sched_exit(-1);
    } else if (token == INFO_TOKEN) {
        ui_menu_about();
    } else if (token == START_RECOVER_TOKEN) {
        wordNum = 0;
        page_keyboard();
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
 * Word recover page
 */
static char textToEnter[MAX_WORD_LENGTH + 1] = {0};
static char headerText[48] = {0};
static nbgl_layout_t *layout = 0;
static int textIndex, suggestionIndex, keyboardIndex = 0;
static char *buttonTexts[NB_MAX_SUGGESTION_BUTTONS] = {0};

// function called when back or any suggestion button is touched
static void layoutTouchCallback(const int token, uint8_t index) {
    if (token == BACK_BUTTON_TOKEN) {
        // go back to main screen of app
        // TODO: instead, back to previous word
        nbgl_layoutRelease(layout);
        display_home_page();
    }
    else if (token >= FIRST_SUGGESTION_TOKEN) {
        // do something with touched button
        PRINTF("Selected word is %s\n", buttonTexts[token - FIRST_SUGGESTION_TOKEN]);

        // go back to main screen of app
        nbgl_layoutRelease(layout);
        display_home_page();
    }
}

// function called when a key of keyboard is touched
static void keyboardCallback(const char touchedKey) {
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

    if (textLen < 2) {
        nbgl_layoutUpdateSuggestionButtons(layout, suggestionIndex, 0, buttonTexts);
    } else {
        const size_t nbMatchingWords = bolos_ux_bip39_fill_with_candidates(
            (unsigned char *)&(textToEnter[0]),
            strlen(textToEnter),
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

void page_keyboard(void) {
    nbgl_layoutDescription_t layoutDescription = {
      .modal = false,
      .onActionCallback = &layoutTouchCallback
    };
    nbgl_layoutKbd_t kbdInfo = {
      .lettersOnly = true,  // use only letters
      .upperCase = false,   // start with lower case letters
      .mode = MODE_LETTERS, // start in letters mode
      .keyMask = 0,         // no inactive key
      .callback = &keyboardCallback
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
    wordNum++;

    layout = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddProgressIndicator(layout, 0, 0, true, BACK_BUTTON_TOKEN, TUNE_TAP_CASUAL);
    snprintf(headerText, 48, "Enter word no.%d from your \nRecovery Sheet", wordNum);
    nbgl_layoutAddCenteredInfo(layout, &centeredInfo);
    keyboardIndex = nbgl_layoutAddKeyboard(layout, &kbdInfo);
    textIndex = nbgl_layoutAddEnteredText(layout,
                                          true,        // numbered
                                          wordNum,     // number to use
                                          textToEnter, // text to display
                                          false,       // not grayed-out
                                          32);         // vertical margin from the buttons
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
        .tapActionToken = START_RECOVER_TOKEN,
        .tuneId = TUNE_TAP_CASUAL
    };
    releaseContext();
    pageContext = nbgl_pageDrawInfo(&pageTouchCallback, NULL, &home);
    nbgl_refresh();
}

#endif


enum UI_STATE { UI_IDLE, UI_TEXT, UI_APPROVAL };

enum UI_STATE uiState;

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

#if defined(TARGET_NANOS)

UX_STEP_CB(restore_3_1_1, bb, G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_24;
           screen_onboarding_4_restore_word_init(RESTORE_WORD_ACTION_FIRST_WORD);
           ,
           {
               "Recovery phrase",
               "with 24 words",
           });

UX_STEP_CB(restore_3_1_2, bb, G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_18;
           screen_onboarding_4_restore_word_init(RESTORE_WORD_ACTION_FIRST_WORD);
           ,
           {
               "Recovery phrase",
               "with 18 words",
           });

UX_STEP_CB(restore_3_1_3, bb, G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_12;
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
            G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_12;
            screen_onboarding_4_restore_word_init(1 /*entering the first word*/);
            break;
        case 1:
            G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_18;
            screen_onboarding_4_restore_word_init(1 /*entering the first word*/);
            break;
        case 2:
            G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_24;
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
    display_home_page();
#endif

}
