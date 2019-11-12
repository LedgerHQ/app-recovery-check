
#include "os.h"
#include "cx.h"

#include "os_io_seproxyhal.h"
#include "string.h"

#if defined(TARGET_NANOS)
    #include "bolos_ux_nanos.h"
#elif defined(TARGET_NANOX)
    #include "bolos_ux_nanox.h"
#endif


#include "glyphs.h"

#ifdef OS_IO_SEPROXYHAL

#define COLOR_BG_1 0xF9F9F9
//#define COLOR_BG_1 0xF80000
//#define ALWAYS_INVERT

#define KEYCODE_SWITCH '\1'
#define KEYCODE_BACKSPACE '\r'

#ifndef SPRINTF
// avoid typing the size each time
#define SPRINTF(strbuf, ...) snprintf((char*)(strbuf), sizeof(strbuf), __VA_ARGS__)
#endif

#define ONBOARDING_CONFIRM_WORD_COUNT 24
#define ONBOARDING_WORD_COMPLETION_MAX_ITEMS 8
#define BOLOS_UX_HASH_LENGTH 4 // as on the blue

#define CONSENT_INTERVAL_MS 3000

extern bolos_ux_context_t G_bolos_ux_context;

extern const unsigned char hex_digits[];

unsigned char rng_u8_modulo(unsigned char modulo);

void screen_hex_identifier_string_buffer (const unsigned char * buffer, unsigned int total);

// common code for all screens
// reset the screen asynch display machine
void screen_state_init(unsigned int stack_slot);

// common code for all screens
// start display of first declared element
void screen_display_init(unsigned int stack_slot);

// request display of the element (taking care of calling screen displayed preprocessors)
void screen_display_element(const bagl_element_t * element);

// prepare to return the exit code after the next display asynch ack is received (0d 00 00)
void screen_return_after_displayed_touched_element(unsigned int exit_code);

void screen_pin_keyboard_init(unsigned int screen_current_element_arrays_index);

// all screens
void screen_saver_init(void);
void screen_saver_deinit(void);

void screen_not_personalized_init(void);
void screen_boot_recovery_init(void);
void screen_dashboard_init(void);
unsigned int screen_is_dashboard(void);
void screen_dashboard_disable_bolos_before_app(void);
void screen_dashboard_prepare(void);
void screen_modal_validate_pin_init(void);
void screen_consent_upgrade_init(void);
void screen_consent_add_init(void);
void screen_consent_del_init(void);
void screen_consent_issuer_key_init(void);
void screen_consent_foreign_key_init(void);
void screen_consent_get_device_name_init(void);
void screen_consent_set_device_name_init(void);
void screen_boot_unsafe_wipe_init(void);
void screen_loader_init(void);
void screen_consent_ux_not_signed_init(void);
void screen_consent_customca_key_init(void);
void screen_consent_reset_customca_init(void);
void screen_consent_setup_customca_init(void);

void screen_random_boarding_init(void);

void screen_onboarding_0_welcome_init(void);
void screen_onboarding_1_2_pin_init(unsigned int step);
void screen_onboarding_3_new_init(void);
void screen_onboarding_4_confirm_init(unsigned int feilword);

void screen_onboarding_3_restore_init(void);
#define RESTORE_WORD_ACTION_REENTER_WORD 0
#define RESTORE_WORD_ACTION_FIRST_WORD 1
void screen_onboarding_4_restore_word_init(unsigned int action);

void screen_onboarding_5_passphrase_init(void);

void screen_onboarding_7_processing_init(void);

void screen_settings_init(unsigned int initial);
// apply settings @ boot time
void screen_settings_apply(void);
void screen_settings_change_pin(void);
void screen_settings_erase_all(void);
void screen_settings_set_temporary(void);
void screen_settings_attach_to_pin(void);
void screen_settings_change_pin_1_2_pin_init(unsigned int initial);
void screen_settings_passphrase_attach_1_init(void);
void screen_settings_passphrase_attach_2_init(unsigned int initial);
void screen_settings_passphrase_temporary_1_init(void);
void screen_settings_passphrase_type_and_review_init(unsigned int kind);
void screen_settings_erase_all_init(void);

#define COMMON_KEYBOARD_INDEX_UNCHANGED (-1UL)

#if defined(TARGET_NANOX)
// to be included into all flow that needs to go back to the dashboard
extern const ux_flow_step_t          ux_ob_goto_dashboard_step;

void ux_pairing_init(void);
void ux_pairing_deinit(void);

void ux_boot_menu_init(void);
void ux_ob_onboarding_init(void);
void screen_control_center_init(unsigned int);

void screen_help_init(appmain_t help_ended_callback);

void screen_processing_init(void);

void screen_prepare_app_name(unsigned int appidx);

bagl_icon_details_t* screen_prepare_bitmap_14x14(ux_icon_buffer_t* icon_buffer, const unsigned char* icon_bitmap, unsigned int masked_inverted);
void screen_prepare_app_icon_14x14(ux_icon_buffer_t* icon_buffer, const unsigned char* icon_bitmap, unsigned int icon_bitmap_length, unsigned int masked_inverted);
void screen_prepare_app_icon(ux_icon_buffer_t* icon_buffer, unsigned char* icon_bitmap_buffer, unsigned int icon_bitmap_buffer_length, unsigned int appidx, unsigned int inverted);

void screen_lock(void);

#ifdef BOLOS_OS_UPGRADER
void screen_os_upgrader(void);
#endif // BOLOS_OS_UPGRADER

unsigned int screen_consent_button(unsigned int button_mask, unsigned int button_mask_counter);
unsigned int screen_consent_ticker(unsigned int ignored);
void screen_consent_ticker_init(unsigned int number_of_steps, unsigned int interval_ms, unsigned int check_pin_to_confirm);
void screen_consent_set_interval(unsigned int interval_ms);

void screen_common_pin_init(unsigned int stack_slot, pin_callback_t end_callback);

void screen_common_keyboard_init(unsigned int stack_slot, unsigned int current_element, unsigned int nb_elements, keyboard_callback_t callback);
void screen_keyboard_init(char* buffer, unsigned int maxsize, appmain_t validation_callback);

void debug(unsigned int id, unsigned char* msg);

void screen_mcu_upgrade_required_init(void);
void screen_consent_run_app_init(void);
void screen_boot_delay_init(void);

void settings_general_version(appmain_t end_callback);
void settings_general_regulatory(appmain_t end_callback);
void settings_general_serial(appmain_t end_callback);
#endif //TARGET_NANOX

#endif // OS_IO_SEPROXYHAL
