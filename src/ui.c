#include <ux.h>
#include "ui.h"

enum UI_STATE { UI_IDLE, UI_TEXT, UI_APPROVAL };

enum UI_STATE uiState;

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

#if defined(TARGET_NANOS)

UX_STEP_CB(restore_1_1_1, bb, G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_24;
           screen_onboarding_restore_word_init(RESTORE_WORD_ACTION_FIRST_WORD);
           ,
           {
               "Recovery phrase",
               "with 24 words",
           });

UX_STEP_CB(restore_1_1_2, bb, G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_18;
           screen_onboarding_restore_word_init(RESTORE_WORD_ACTION_FIRST_WORD);
           ,
           {
               "Recovery phrase",
               "with 18 words",
           });

UX_STEP_CB(restore_1_1_3, bb, G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_12;
           screen_onboarding_restore_word_init(RESTORE_WORD_ACTION_FIRST_WORD);
           ,
           {
               "Recovery phrase",
               "with 12 words",
           });

UX_FLOW(restore_1_1, &restore_1_1_1, &restore_1_1_2, &restore_1_1_3);

void screen_onboarding_1_restore_init(void) {
    G_bolos_ux_context.onboarding_type = BOLOS_UX_ONBOARDING_BIP39;
    ux_flow_init(0, restore_1_1, NULL);
}

void screen_onboarding_2_restore_init(void) {
    G_bolos_ux_context.onboarding_type = BOLOS_UX_ONBOARDING_SSKR;
    screen_onboarding_restore_word_init(RESTORE_WORD_ACTION_FIRST_WORD);
}

UX_STEP_VALID(ux_idle_flow_1_step, pbb, screen_onboarding_1_restore_init();,
                                                                           {
                                                                               &C_badge,
                                                                               "Check BIP39",
                                                                               "recovery phrase",
                                                                           });

UX_STEP_VALID(ux_idle_flow_2_step, pbb, screen_onboarding_2_restore_init();
              ,
              {
                  &C_nanos_app_sskr_check,
                  "Check SSKR",
                  "recovery phrase",
              });

#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)

//////////////////////////////////////////////////////////////////////

const char* const number_of_bip39_words_values[] = {
    "12 words",
    "18 words",
    "24 words",
    "Back",
};

const char* number_of_bip39_words_get(unsigned int idx) {
    if (idx < ARRAYLEN(number_of_bip39_words_values)) {
        return number_of_bip39_words_values[idx];
    }
    return NULL;
}

void number_of_bip39_words_selector(unsigned int idx) {
    switch (idx) {
        case 0:
            G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_12;
            goto word_init;
        case 1:
            G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_18;
            goto word_init;
        case 2:
            G_bolos_ux_context.onboarding_kind = BOLOS_UX_ONBOARDING_NEW_24;
            goto word_init;
        word_init:
            screen_onboarding_restore_word_init(RESTORE_WORD_ACTION_FIRST_WORD);
            break;
        default:
            ui_idle_init();
    }
}

void screen_onboarding_1_restore_init(void) {
    G_bolos_ux_context.onboarding_type = BOLOS_UX_ONBOARDING_BIP39;
    ux_menulist_init(0, number_of_bip39_words_get, number_of_bip39_words_selector);
}

void screen_onboarding_2_restore_init(void) {
    G_bolos_ux_context.onboarding_type = BOLOS_UX_ONBOARDING_SSKR;
    screen_onboarding_restore_word_init(RESTORE_WORD_ACTION_FIRST_WORD);
}

//////////////////////////////////////////////////////////////////////

UX_STEP_VALID(ux_instruction_1_1_step,
              nnn,
              screen_onboarding_1_restore_init(),
              {
                  "Select the number of",
                  "words written on",
                  "your Recovery Sheet",
              });

UX_FLOW(ux_instruction_1_flow, &ux_instruction_1_1_step);

UX_STEP_VALID(ux_instruction_2_1_step,
              nnn,
              screen_onboarding_2_restore_init(),
              {
                  "Enter first word of",
                  "first share of SSKR",
                  "recovery phrase",
              });

UX_FLOW(ux_instruction_2_flow, &ux_instruction_2_1_step);

//////////////////////////////////////////////////////////////////////

UX_STEP_VALID(ux_idle_flow_1_step,
              pbb,
              ux_flow_init(0, ux_instruction_1_flow, NULL),
              {
                  &C_badge,
                  "Check BIP39",
                  "recovery phrase",
              });
UX_STEP_VALID(ux_idle_flow_2_step,
              pbb,
              ux_flow_init(0, ux_instruction_2_flow, NULL),
              {
                  &C_nanox_app_sskr_check,
                  "Check SSKR",
                  "recovery phrase",
              });
#endif

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
UX_FLOW(ux_idle_flow,
        &ux_idle_flow_1_step,
        &ux_idle_flow_2_step,
        &ux_idle_flow_3_step,
        &ux_idle_flow_4_step);

void ui_idle_init(void) {
    uiState = UI_IDLE;

    // reserve a display stack slot if none yet
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_idle_flow, NULL);
}
