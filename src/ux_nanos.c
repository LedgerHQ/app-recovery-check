#include "ux_nanos.h"

enum UI_STATE { UI_IDLE, UI_TEXT, UI_APPROVAL };

enum UI_STATE uiState;



extern const ux_menu_entry_t menu_about_nanos[];
extern const ux_menu_entry_t ui_idle_mainmenu_nanos[];


const ux_menu_entry_t menu_about_nanos[] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {ui_idle_mainmenu_nanos, NULL, 1, &C_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END}; 

const ux_menu_entry_t ui_idle_mainmenu_nanos[] = {
  {NULL, screen_onboarding_3_restore_init, 0, &C_badge, "Check your", "seed", 32, 10},
  //{/*menu_settings_nanos*/NULL, NULL, 0, NULL, "Settings", NULL, 0, 0},
  {menu_about_nanos, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
}; 


void ui_idle_init(void) {
  uiState = UI_IDLE;

  UX_MENU_DISPLAY(0, ui_idle_mainmenu_nanos, NULL);
  // setup the first screen changing
  UX_CALLBACK_SET_INTERVAL(1000);
}