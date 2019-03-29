
#ifndef BOLOS_UX_H
#define BOLOS_UX_H

#include "os_io_seproxyhal.h"

#ifdef HAVE_BOLOS_UX

#include "ux.h"

typedef unsigned int (*pin_callback_t) (unsigned char* pin_buffer, unsigned int pin_length);

#define KEYBOARD_ITEM_VALIDATED 1 // callback is called with the entered item index, tmp_element is precharged with element to be displayed and using the common string buffer as string parameter
#define KEYBOARD_RENDER_ITEM    2 // callback is called the element index, tmp_element is precharged with element to be displayed and using the common string buffer as string parameter
#define KEYBOARD_RENDER_WORD    3 // callback is called with a -1 when requesting complete word, or the char index else, returnin 0 implies no char is to be displayed
typedef const bagl_element_t* (*keyboard_callback_t) (unsigned int event, unsigned int value);


#define ICON_14x14_SIZE_B (UPPER_ALIGN(GLYPH_dashboard_mask_WIDTH*GLYPH_dashboard_mask_HEIGHT, 8, unsigned int) / 8)

typedef struct {
  bagl_icon_details_t details;
  unsigned char bitmap[ICON_14x14_SIZE_B];
} ux_icon_buffer_t;

#define APPLICATION_MAXCOUNT 100

// bolos ux context (not mandatory if redesigning a bolos ux)
typedef struct bolos_ux_context {

  #define STATE_UNINITIALIZED 0
#ifndef STATE_INITIALIZED
  #define STATE_INITIALIZED 0xB01055E5UL
#endif // STATE_INITIALIZED
  unsigned int state; 

  // a screen pop occured, the underlaying screen must optimize its drawing as we've probably trashed the whole screen
  unsigned int screen_redraw;

#define BATTERY_CHECK_INTERVAL_MS 5000
  unsigned int ms_last_batt_check;

  unsigned int ms;
  unsigned int setting_auto_lock_delay_ms;
  unsigned int setting_power_off_delay_ms;
  
#define AUTO_LOCK_DEFAULT 300000
#define POWER_OFF_DEFAULT 600000

#define IS_SETTING_PRE_POWER_OFF() (G_bolos_ux_context.setting_auto_lock_delay_ms != -1UL && G_bolos_ux_context.setting_auto_lock_delay_ms != 0)
#define IS_SETTING_POWER_OFF() (G_bolos_ux_context.setting_power_off_delay_ms != -1UL && G_bolos_ux_context.setting_power_off_delay_ms != 0)
#define INACTIVITY_MS_AUTO_LOCK  (G_bolos_ux_context.setting_auto_lock_delay_ms)
  #define INACTIVITY_MS_POWER_OFF  (/*G_bolos_ux_context.setting_auto_lock_delay_ms+*/G_bolos_ux_context.setting_power_off_delay_ms)
  unsigned int ms_last_activity;
  unsigned int ms_last_activity_saver;
  unsigned int ms_last_activity_pwroff;

  unsigned int last_button_state;

  // control center display push both buttons delay
#define BOLOS_CC_PERIOD_MS 1500UL
  unsigned int ms_cc_start;

  enum {
    INACTIVITY_NONE,
    INACTIVITY_LOCK,
    INACTIVITY_SAVER,
  } inactivity_state;

  unsigned int control_center_entering;

  unsigned int last_ux_id;

#define BOLOS_UX_ONBOARDING_NEW 1
#define BOLOS_UX_ONBOARDING_NEW_12 12
#define BOLOS_UX_ONBOARDING_NEW_18 18
#define BOLOS_UX_ONBOARDING_NEW_24 24  
#define BOLOS_UX_ONBOARDING_RESTORE 2
#define BOLOS_UX_ONBOARDING_RESTORE_12 12
#define BOLOS_UX_ONBOARDING_RESTORE_18 18
#define BOLOS_UX_ONBOARDING_RESTORE_24 24
  unsigned int onboarding_kind;

#ifdef HAVE_ELECTRUM
  unsigned int onboarding_algorithm;
#endif

  unsigned int onboarding_step;
  unsigned int onboarding_index;
  unsigned int onboarding_words_checked;
  unsigned int onboarding_words_are_valid;
  unsigned int onboarding_step_checked_inc;
  unsigned int onboarding_step_checked;



  unsigned int words_buffer_length;
  // 128 of words (215 => hashed to 64, or 128) + HMAC_LENGTH*2 = 256
#define WORDS_BUFFER_MAX_SIZE_B 257
  char words_buffer[WORDS_BUFFER_MAX_SIZE_B]; 
#define MAX_PIN_LENGTH 8
#define MIN_PIN_LENGTH 4
  pin_callback_t pin_end_callback;
  char pin_buffer[MAX_PIN_LENGTH+1]; // length prepended for custom pin length
  char pin_digit_buffer; // digit to be displayed

  appmain_t flow_end_callback;

  // unsigned int settings_index;
  // unsigned int settings_value;

  bagl_element_t saver_element;
  // int saver_step_x;
  // int saver_step_y;
#define SAVER_ELEMENTS 8
  struct {
    int step_x;
    int step_y;
    int x;
    int y;
  } savers[SAVER_ELEMENTS];

  // label line for common PIN and common keyboard screen (displayed over the entry)
  const char* common_label;

  unsigned int pairing_stack_slot_plus_1;

  // slider management / menu list management
  unsigned int hslider3_before;
  unsigned int hslider3_current;
  unsigned int hslider3_after;
  unsigned int hslider3_total;

  keyboard_callback_t keyboard_callback;
  unsigned char keyboard_user_buffer[BOLOS_UX_KEYBOARD_TEXT_BUFFER_SIZE+1];

  unsigned int last_installed_os_index;

  unsigned int control_center_enabled;
  unsigned int control_center_current;
  unsigned int control_center_total;
  unsigned char* control_center_mapping;
  unsigned char control_center_string[MAX(128,sizeof(bagl_icon_details_t)-1)];

  unsigned int dashboard_current; // index of the currently selected icon on the dashboard
  unsigned int dashboard_previous; // index of the previously selected icon on the dashboard
  unsigned int dashboard_first; // index of the first item on the top line displayed on the dashboard
  unsigned int dashboard_total;
  unsigned char dashboard_sorted_app_os_idx[APPLICATION_MAXCOUNT];
  unsigned int dashboard_last_selected;
  //unsigned int dashboard_redisplayed; // to trigger animation when all elements are displayed
  //unsigned int dashboard_no_apps_displayed; // when the message saying no apps installed has been displayed or not
  // in case autostart is engaged, to avoid starting the app multiple times
  unsigned int app_auto_started;  

  unsigned int overlay_refresh;
  unsigned int battery_percentage;
  unsigned int status_batt_level;
  unsigned int status_flags;
  unsigned int batt_low_displayed;
  unsigned int batt_critical_displayed;


#define BATTERY_FULL_CHARGE_MV          4200 // 100%
#define BATTERY_SUFFICIENT_CHARGE_MV    3840 //  40%
#define BATTERY_LOW_LEVEL_MV            3750 //  25%
#define BATTERY_CRITICAL_LEVEL_MV       3460 //  10%
#define BATTERY_AUTO_POWER_OFF_LEVEL_MV 3200 //   0%

#define BATTERY_FULL_CHARGE_PERCENT          95
#define BATTERY_SUFFICIENT_CHARGE_PERCENT    40
#define BATTERY_LOW_LEVEL_PERCENT            25
#define BATTERY_CRITICAL_LEVEL_PERCENT       10
#define BATTERY_AUTO_POWER_OFF_LEVEL_PERCENT  2

  // to return to the current context after help screens have been validated
  appmain_t help_ended_callback;
  unsigned int help_screen_idx;

  // detect stack/global variable overlap 
  // have a zero byte to avoid buffer overflow from strings in the ux (we never know)
#define CANARY_MAGIC 0x7600E9AB
  unsigned int canary;

  // for CheckSeed app only
  uint8_t input_seed_is_identical;
  uint8_t processing;

} bolos_ux_context_t;

extern bolos_ux_context_t G_bolos_ux_context;

void ux_stack_display_elements(ux_stack_slot_t* slot); // not to be displayed in the SDK

// update before, current, after index for horizontal slider with 3 positions
// slider distinguish handling from the data, to be more generic :)
#define BOLOS_UX_HSLIDER3_NONE (-1UL)
void bolos_ux_hslider3_init(unsigned int total_count);
void bolos_ux_hslider3_set_current(unsigned int current);
void bolos_ux_hslider3_next(void);
void bolos_ux_hslider3_previous(void);

#define FAST_LIST_THRESHOLD_CS 8
#define FAST_LIST_ACTION_CS    2

/**
 * Bolos system app internal UX entry point (could be overriden by a further loaded BOLOS_UX application)
 */
void bolos_ux_main(void);

#include "bolos_ux_common_bip39.h"


#define UX_FLOW_AFTER_PIN(stepname, stackslot, callback) \
 UX_STEP_INIT( \
  stepname, \
  NULL, \
  NULL, \
  { \
    /* invalidate pin and display pin lock */ \
    screen_modal_validate_pin_init(); \
    /* display processing screen on slot 0 (pin modal has been pushed over) */ \
    /* after display then perform */ \
    ux_stack_init(stackslot); \
    G_ux.stack[stackslot].displayed_callback = callback; \
  });

#endif // HAVE_BOLOS_UX

#endif // BOLOS_UX_H
