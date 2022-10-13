#include <ux.h>
#include "ui.h"

#if defined(HAVE_NBGL)

#include <nbgl_fonts.h>
#include <nbgl_front.h>
#include <nbgl_debug.h>
#include <nbgl_page.h>

nbgl_page_t* pageContext;

void ui_menu_about();
void recover_page();

void releaseContext(void) {
    if (pageContext != NULL) {
        nbgl_pageRelease(pageContext);
        pageContext = NULL;
    }
}

enum { BACK_TOKEN = 0, INFO_TOKEN, NEXT_TOKEN, CANCEL_TOKEN, START_RECOVER_TOKEN, BACK_HOME_TOKEN, QUIT_APP_TOKEN };

void pageTouchCallback(int token, uint8_t index) {
    (void) index;
    PRINTF("LOL");
    if (token == QUIT_APP_TOKEN) {
        releaseContext();
        os_sched_exit(-1);
    } else if (token == INFO_TOKEN) {
        ui_menu_about();
    } else if (token == START_RECOVER_TOKEN) {
        recover_page();
    } else if (token == BACK_HOME_TOKEN) {
        releaseContext();
        ui_idle_init();
    }
}

// 'About' menu

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

void recover_page() {

    nbgl_layoutDescription_t layoutDescription = {
        .modal = false, // not modal (so on plane 0)
        .onActionCallback = NULL, // generic callback for all controls
        .tapActionText = "Return", // A "tapable" main container is necessary, with this text
        .tapActionToken = BACK_HOME_TOKEN, // token to be used when main container is touched
        .ticker.tickerCallback = NULL // no ticker
    };

    nbgl_layoutProgressBar_t bar = {
        .percentage = 50,
        .text = "coucou lol",
        .subText = "sub"
    };

    nbgl_layout_t *layout = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddKeyboard(layout, &bar);

    releaseContext();
    nbgl_layoutDraw(layout);
    nbgl_refresh();
}

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
