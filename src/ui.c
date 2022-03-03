#include "ui.h"

enum UI_STATE { UI_IDLE, UI_TEXT, UI_APPROVAL };

enum UI_STATE uiState;

#if defined(TARGET_NANOS)

UX_STEP_VALID(
    ux_idle_flow_1_step, 
    pbb,
    screen_onboarding_3_restore_init();,
    {
      &C_badge,
      "Check your",
      "recovery phrase",
    });
UX_STEP_NOCB(
    ux_idle_flow_3_step, 
    bn, 
    {
      "Version",
      APPVERSION,
    });
UX_STEP_VALID(
    ux_idle_flow_4_step,
    pb,
    os_sched_exit(-1),
    {
      &C_icon_dashboard_x,
      "Quit",
    });
UX_FLOW(ux_idle_flow,
  &ux_idle_flow_1_step,
  &ux_idle_flow_3_step,
  &ux_idle_flow_4_step
);

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
  switch(idx) {
    case 0:
      G_bolos_ux_context.onboarding_kind = 12;
      screen_onboarding_4_restore_word_init(1 /*entering the first word*/);
      break;
    case 1:
      G_bolos_ux_context.onboarding_kind = 18;
      screen_onboarding_4_restore_word_init(1 /*entering the first word*/);
      break;
    case 2:
      G_bolos_ux_context.onboarding_kind = 24;
      screen_onboarding_4_restore_word_init(1 /*entering the first word*/);
      break;
    default:
      ui_idle_init();
  }
}

//////////////////////////////////////////////////////////////////////

UX_STEP_VALID(
    ux_instruction_step, 
    nnn,
    ux_menulist_init(0, number_of_words_getter, number_of_words_selector),
    {
      "Select the number",
      "of words written on",
      "your Recovery Sheet",
    });

UX_FLOW(ux_instruction_flow,
  &ux_instruction_step
);

//////////////////////////////////////////////////////////////////////

UX_STEP_VALID(
    ux_idle_flow_1_step, 
    pbb,
    ux_flow_init(0, ux_instruction_flow, NULL),
    {
      &C_badge,
      "Check your",
      "recovery phrase",
    });
UX_STEP_NOCB(
    ux_idle_flow_3_step, 
    bn, 
    {
      "Version",
      APPVERSION,
    });
UX_STEP_VALID(
    ux_idle_flow_4_step,
    pb,
    os_sched_exit(-1),
    {
      &C_icon_dashboard_x,
      "Quit",
    });
UX_FLOW(ux_idle_flow,
  &ux_idle_flow_1_step,
  &ux_idle_flow_3_step,
  &ux_idle_flow_4_step
);

#endif


void ui_idle_init(void) {
  uiState = UI_IDLE;

  // reserve a display stack slot if none yet
  if(G_ux.stack_count == 0) {
      ux_stack_push();
  }
  ux_flow_init(0, ux_idle_flow, NULL);
}
