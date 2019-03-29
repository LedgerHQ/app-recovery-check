#include "os.h"

#ifdef TARGET_NANOX

#include "cx.h"
#include "ux.h"

#include "os_io_seproxyhal.h"
#include "string.h"

#include "bolos_ux_common.h"
#include "glyphs.h"

#ifdef OS_IO_SEPROXYHAL

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

bolos_ux_context_t G_bolos_ux_context;


#define DEFAULT_PIN "5555"
#define DEFAULT_PIN_ALT "4444"

#if defined(BOLOS_FIXED_ONBOARDING) || defined(HAVE_BOLOS_NOSCREEN)
#define DEFAULT_SEED "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about"

//#define DEFAULT_SEED "sunny soap dice hub oven pupil security unlock used spend jacket boat jump asset afford rhythm doll hidden sand domain total until horror sock"
// feiling too // #define DEFAULT_SEED "sunny soap dice hub oven pupil security unlock used spend jacket boat jump asset afford rhythm doll hidden sand domain total until"
// working // #define DEFAULT_SEED "sunny soap dice hub oven pupil security unlock used spend jacket boat jump asset afford rhythm doll hidden sand domain total"
void os_fixed_onboarding(void) {
  
  G_bolos_ux_context.words_buffer_length = strlen(DEFAULT_SEED);
  os_memmove(G_bolos_ux_context.words_buffer, DEFAULT_SEED, G_bolos_ux_context.words_buffer_length);

  // store the pin
  os_perso_set_pin(0, DEFAULT_PIN, 4);

  os_perso_derive_and_set_seed(0, 
                             NULL, 0,
                             NULL, 0,
                             G_bolos_ux_context.words_buffer, G_bolos_ux_context.words_buffer_length);

  // seal the fake onboarding
  os_perso_finalize();

  // unlock to be able to set the next identity
  os_global_pin_check((unsigned char*)DEFAULT_PIN, 4);

  // setup an alternate seed with a pin
  os_perso_set_pin(1, DEFAULT_PIN_ALT, 4);
  os_perso_derive_and_set_seed(1, 
                               NULL, 0,
                               "passphrase", 10,
                               G_bolos_ux_context.words_buffer, G_bolos_ux_context.words_buffer_length);
}
#else
void os_random_onboarding(void) {
  
  os_memset(G_bolos_ux_context.words_buffer, 0, sizeof(G_bolos_ux_context.words_buffer));
  cx_rng((unsigned char*)G_ux.string_buffer, 32);
  G_bolos_ux_context.words_buffer_length = bolos_ux_mnemonic_from_data((unsigned char*)G_ux.string_buffer, 
                                                                       32, 
                                                                       (unsigned char*)G_bolos_ux_context.words_buffer, 
                                                                       sizeof(G_bolos_ux_context.words_buffer));

  // store the pin
  os_perso_set_pin(0, (unsigned char*)DEFAULT_PIN, 4);

  // store the seed and the word list
  os_perso_set_words((unsigned char*)G_bolos_ux_context.words_buffer, G_bolos_ux_context.words_buffer_length); // word buffer can serve as a work buffer afterwards

  // TODO store the device's name

  // pbkdf2 the seed
  // TODO try/catch me
  bolos_ux_mnemonic_to_seed((unsigned char*)G_bolos_ux_context.words_buffer, G_bolos_ux_context.words_buffer_length,
                            (unsigned char*)G_ux.string_buffer);
  
  // Store the derived seed
  os_perso_set_seed(0, BOLOS_UX_ONBOARDING_ALGORITHM_BIP39, (unsigned char*)G_ux.string_buffer, 64);

  // seal the fake onboarding
  os_perso_finalize();

  // validate pin
  os_global_pin_check((unsigned char*)DEFAULT_PIN, 4);
}

#endif // BOLOS_FIXED_ONBOARDING || HAVE_BOLOS_NOSCREEN

unsigned short io_timeout(unsigned short last_timeout) {
  UNUSED(last_timeout);
  // infinite timeout
  return 1;
}


void screen_hex_identifier_string_buffer (const unsigned char * buffer, unsigned int total) {
  SPRINTF(G_ux.string_buffer, 
            "%.*H...%.*H", 
            BOLOS_UX_HASH_LENGTH/2, 
            buffer, 
            BOLOS_UX_HASH_LENGTH/2, 
            buffer
              +total
              -BOLOS_UX_HASH_LENGTH/2);
}

const bagl_element_t screen_overlay_elements[] = {
  // erase overlay
  {{BAGL_RECTANGLE                      , 0x00, BAGL_WIDTH-8,   0, 8,  12, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0}, NULL, 0, 0, 0, NULL, NULL, NULL},
  // batt layout (empty, just need a rectangle to fill)
  {{BAGL_ICON                           , 0x00, BAGL_WIDTH-8,   0, 8,  12, 0, 0, 0        , 0xFFFFFF, 0x000000, 0, 0}, (const char*)&C_icon_bat_ovl_critical, 0, 0, 0, NULL, NULL, NULL },
  // batt inner rectangle
  {{BAGL_RECTANGLE                      , 0x00, BAGL_WIDTH-8+3, 3, 3,   6, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0}, NULL, 0, 0, 0, NULL, NULL, NULL},
  // charging icon
  {{BAGL_ICON                           , 0x00, BAGL_WIDTH-3,   0, 3,   6, 0, 0, 0        , 0xFFFFFF, 0x000000, 0, 0}, (const char*)&C_icon_bat_ovl_charging, 0, 0, 0, NULL, NULL, NULL },
  // plugged icon
  {{BAGL_ICON                           , 0x00, BAGL_WIDTH-8,   0, 8,  12, 0, 0, 0        , 0xFFFFFF, 0x000000, 0, 0}, (const char*)&C_icon_bat_ovl_plug, 0, 0, 0, NULL, NULL, NULL },
};

void screen_display_overlay(void) {
  // disable the keep out zone
  screen_set_keepout(0, 0, 0, 0);
  // erase
  io_seproxyhal_display_default(&screen_overlay_elements[0]);
  
  // battery state
  // NOTE: don't display when percentage is not yet correctly computed by STC3115
  // not charging and plugged
  if ((G_bolos_ux_context.status_flags & (SEPROXYHAL_TAG_STATUS_EVENT_FLAG_CHARGING|SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) == SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED) {
    io_seproxyhal_display_default(&screen_overlay_elements[4]);
  }
  // charging or not plugged
  // if battery level is somewhat intelligible
  else if (G_bolos_ux_context.battery_percentage != 0xFFu) {
    // draw the battery frame
    io_seproxyhal_display_default(&screen_overlay_elements[1]);
    // fill with current battery level
    os_memmove(&G_ux.tmp_element, &screen_overlay_elements[2], sizeof(bagl_element_t));

    unsigned int batpix = MAX(1,(UPPER_ALIGN(G_bolos_ux_context.battery_percentage, 14, char)/14))-1;
    G_ux.tmp_element.component.y += 6-batpix;
    G_ux.tmp_element.component.height = batpix;
    io_seproxyhal_display_default(&G_ux.tmp_element);
    // plugged and charging ?
    if (G_bolos_ux_context.status_flags & (SEPROXYHAL_TAG_STATUS_EVENT_FLAG_CHARGING|SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED) == (SEPROXYHAL_TAG_STATUS_EVENT_FLAG_CHARGING|SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
      io_seproxyhal_display_default(&screen_overlay_elements[3]);
    }
  }
  else {
    // plugged and charging ?
    if (G_bolos_ux_context.status_flags & (SEPROXYHAL_TAG_STATUS_EVENT_FLAG_CHARGING|SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED) == (SEPROXYHAL_TAG_STATUS_EVENT_FLAG_CHARGING|SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
      io_seproxyhal_display_default(&screen_overlay_elements[4]);
    }
  }

  screen_set_keepout(BAGL_WIDTH-8, 0, 8, 12);
  screen_update();
}


void screen_display_element(const bagl_element_t * element) {
  const bagl_element_t* el = ux_stack_display_element_callback(element);
  if (!el) {
    return;
  }
  if ((unsigned int)el != 1) {
    element = el;
  }
  // display current element
  io_seproxyhal_display(element);
}

void screen_wake_up(void) {
  // only reactivate backlight when dimmed, to avoid blink ...b
  if (G_bolos_ux_context.inactivity_state == INACTIVITY_SAVER) {
    // not inactive anymore, interpret touch/button
    // wake backlight, don't touch the current state
    //io_seproxyhal_backlight(0, BACKLIGHT_FULL_LEVEL);
#ifndef BOLOS_NO_SCREENLOCK
    screen_saver_deinit();
    // // mark as unlocked
    G_bolos_ux_context.inactivity_state = INACTIVITY_LOCK;
    // simulate a user action, it sure is delaying the power off, but avoid going lock right after redisplaying the pin, therefore avoiding a glitch
    G_bolos_ux_context.ms_last_activity_saver = G_bolos_ux_context.ms;
#endif // !BOLOS_NO_SCREENLOCK
  }
}

void screen_lock(void) {
  // ensure locked before saver is displayed
  screen_modal_validate_pin_init();
  
  // prepare the lock screen
  G_bolos_ux_context.inactivity_state = INACTIVITY_LOCK;
}

void screen_save(void) {
  // yay, some fun, ensure saver is stacked over the pin
  screen_saver_init();
  // enter saver state
  G_bolos_ux_context.inactivity_state = INACTIVITY_SAVER;
}

unsigned int screen_is_not_locked(void) {
  return os_global_pin_is_validated() == BOLOS_UX_OK || os_perso_isonboarded() != BOLOS_UX_OK;
}

void screen_return_after_displayed_touched_element(unsigned int exit_code) {
  G_ux.stack[G_ux.stack_count-1].element_index = 0;
  G_ux.stack[G_ux.stack_count-1].displayed = 0;
  G_ux.stack[G_ux.stack_count-1].element_arrays_count = 0;
  G_ux.stack[G_ux.stack_count-1].exit_code_after_elements_displayed = exit_code;
}

unsigned char const C_dashboard_placeholder_bytes[] = {
0x01, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x30, 0x83, 0x01, 0x26, 0x00, 0x01, 0x00, 0x01, 0x60, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x60, 0x00, 0x08, 0x00, 0x08, 0x40, 0x06, 0x18, 0xcc, 0x00, };

/**
 * requires an out buffer of sizeof(bagl_icon_details_t)+ICON_14x14_SIZE_B
 */
bagl_icon_details_t* screen_prepare_bitmap_14x14(ux_icon_buffer_t* icon_buffer, const unsigned char* icon_bitmap, unsigned int masked_inverted) {
  unsigned int i;

  icon_buffer->details.width = 14;
  icon_buffer->details.height = 14;
  // prepare the icon_details content
  icon_buffer->details.bpp = 1;
  // use color table from the const, take care of the alignment here, but well the the color 
  // array is not deref as a uint32 as is afterward i nthe process, so no problem
  icon_buffer->details.colors = (unsigned int*) C_dashboard_mask_colors;
  icon_buffer->details.bitmap = &icon_buffer->bitmap;


  // empty masks if the bitmap format does not fit
  if (masked_inverted) {
    // initialise with the mask if icon
    os_memmove(&icon_buffer->bitmap[0], C_dashboard_mask_bitmap, ICON_14x14_SIZE_B);
  }
  else {
    os_memset(&icon_buffer->bitmap[0], 0, ICON_14x14_SIZE_B);
  }


  if (icon_bitmap == NULL) {
    // use a placeholder
    icon_bitmap = C_dashboard_placeholder_bytes+9;
  }

  // no inversemode supporte,d if bitmap color map is not with 0 first, then image has to be reencoded sry

  for (i=0; i<(ICON_14x14_SIZE_B); i++) {
    if (masked_inverted) {
      icon_buffer->bitmap[i] = C_dashboard_mask_bitmap[i] & (~icon_bitmap[i]);
    }
    else {
      icon_buffer->bitmap[i] = C_dashboard_mask_bitmap[i] & icon_bitmap[i];
    }
  }

  return &icon_buffer->details;
}

// prepare the app icon as if it was a icon_detail_t encoded structure in given out buffer
// @param masked_inverted means the icon is displayed within a white rounded corner rectangle
void screen_prepare_app_icon_14x14(ux_icon_buffer_t* icon_buffer, const unsigned char* icon_bitmap, unsigned int icon_bitmap_length, unsigned int masked_inverted) {
  unsigned int i, inversemode;

  icon_buffer->details.width = 14;
  icon_buffer->details.height = 14;
  // prepare the icon_details content
  icon_buffer->details.bpp = 1;
  // use color table from the const, take care of the alignment here, but well the the color 
  // array is not deref as a uint32 as is afterward i nthe process, so no problem
  icon_buffer->details.colors = (unsigned int*) C_dashboard_mask_colors;
  icon_buffer->details.bitmap = &icon_buffer->bitmap;


  // empty masks if the bitmap format does not fit
  if (masked_inverted) {
    // initialise with the mask if icon
    os_memmove(&icon_buffer->bitmap[0], C_dashboard_mask_bitmap, ICON_14x14_SIZE_B);
  }
  else {
    os_memset(&icon_buffer->bitmap[0], 0, ICON_14x14_SIZE_B);
  }

  if (icon_bitmap_length<9 
    || icon_bitmap_length > 9+(ICON_14x14_SIZE_B) 
    || !icon_bitmap 
    || icon_bitmap[0] != 1 /*bpp*/) {
    // use a placeholder
    icon_bitmap = C_dashboard_placeholder_bytes;
    icon_bitmap_length = sizeof(C_dashboard_placeholder_bytes);
  }

  // when first color of the bitmap is not 0, then, must inverse the icon's bit to 
  // match the C_dashboard_mask_bitmap bit value 
  inversemode = 0;
  if (icon_bitmap[1] != 0 || icon_bitmap[2] != 0 || icon_bitmap[3] != 0 || icon_bitmap[4] != 0 ) {
    inversemode = 1;
  }

  // the icon bitmap is prefixed with the color index
  icon_bitmap_length-=9;
  icon_bitmap += 9;
  for (i=0; i<(ICON_14x14_SIZE_B) && i < icon_bitmap_length; i++) {
    if (masked_inverted) {
      if (inversemode) {
        icon_buffer->bitmap[i] = C_dashboard_mask_bitmap[i] & icon_bitmap[i];
      } else {
        icon_buffer->bitmap[i] = C_dashboard_mask_bitmap[i] & (~icon_bitmap[i]);
      }
    }
    else {
      icon_buffer->bitmap[i] = C_dashboard_mask_bitmap[i] & icon_bitmap[i];
    }
  }
}

void screen_prepare_app_icon(ux_icon_buffer_t* icon_buffer, unsigned char* icon_bitmap_buffer, unsigned int icon_bitmap_buffer_length, unsigned int appidx, unsigned int inverted) {
  screen_prepare_app_icon_14x14(icon_buffer, icon_bitmap_buffer, 
                       os_registry_get_tag(appidx, NULL, 
                                           BOLOS_TAG_ICON, 
                                           0,
                                           icon_bitmap_buffer, 
                                           MAX(0, icon_bitmap_buffer_length)),
                       inverted);
}

void screen_prepare_app_name(unsigned int appidx) {
  // mark the EOS
  G_ux.string_buffer[os_registry_get_tag(appidx, NULL, BOLOS_TAG_APPNAME, 0, G_ux.string_buffer, BOLOS_APPNAME_MAX_SIZE_B)]=0;
}

void io_seproxyhal_display(const bagl_element_t * element) {
  io_seproxyhal_display_default((bagl_element_t*)element);
}

void screen_keyboard_validate_entered_text(void) {
  // we're returning the typed text
  G_ux.exit_code = BOLOS_UX_OK;
  // last keycode is \n
  G_ux_params.u.keyboard.keycode = '\n';
  // copy output text
  os_memmove(G_ux_params.u.keyboard.entered_text, G_bolos_ux_context.keyboard_user_buffer, sizeof(G_ux_params.u.keyboard.entered_text));
  // pop the screen
  ux_stack_pop();
  ux_stack_redisplay();
}

void bolos_ux_main(void) {
  for (;;) {
BEGIN_TRY {
  TRY {
    unsigned int previous_screen_stack_count;

    os_ux_read_parameters(&G_ux_params);

    // ensure default return code
    G_ux.exit_code = BOLOS_UX_CONTINUE;

    switch(G_bolos_ux_context.state) {
      default:

        // init seproxyhal ux related globals
        io_seproxyhal_init_ux();
        // no button push so far
        io_seproxyhal_init_button();

        // init the ram context
        os_memset(&G_bolos_ux_context, 0, sizeof(G_bolos_ux_context));
        // setup the ram canary
        G_bolos_ux_context.canary = CANARY_MAGIC;
        G_bolos_ux_context.state = STATE_INITIALIZED;
        G_bolos_ux_context.dashboard_last_selected = -1U; // initialize the current selected application to none., done only at boot
        G_bolos_ux_context.battery_percentage =  0xFFu; // ensure battery level is displayed the first time

        // return, this should be the first and only call from the bolos task at platform startup
        G_ux.exit_code = BOLOS_UX_OK;

        break;

      case STATE_INITIALIZED:
        // save the fact that a screen exists currently
        previous_screen_stack_count = G_ux.stack_count;

        // push the default screen to display the ux into it
        if (G_ux.stack_count == 0 
          // no need for a new stacked screen in the following cases (no screen frame needed on top of apps for these calls)
          
          //BEGIN BOLOS MANAGER FLOW (use slot 0 implicitely)
          && (G_ux_params.ux_id == BOLOS_UX_BOOT_NOT_PERSONALIZED
            || G_ux_params.ux_id == BOLOS_UX_BOOT_ONBOARDING
            || G_ux_params.ux_id == BOLOS_UX_DASHBOARD 
            || G_ux_params.ux_id == BOLOS_UX_KEYBOARD
            || G_ux_params.ux_id == BOLOS_UX_LOADER
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_UPGRADE
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_APP_ADD
          //|| G_ux_params.ux_id == BOLOS_UX_CONSENT_APP_UPG
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_APP_DEL
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_ISSUER_KEY
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_CUSTOMCA_KEY
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_FOREIGN_KEY
          //|| G_ux_params.ux_id == BOLOS_UX_CHANGE_ALTERNATE_PIN
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_GET_DEVICE_NAME
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_SET_DEVICE_NAME
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_SETUP_CUSTOMCA_KEY
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_RESET_CUSTOMCA_KEY
            || G_ux_params.ux_id == BOLOS_UX_BOOT_UX_NOT_SIGNED
            || G_ux_params.ux_id == BOLOS_UX_PROCESSING
            || G_ux_params.ux_id == BOLOS_UX_MCU_UPGRADE_REQUIRED
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_RUN_APP
            || G_ux_params.ux_id == BOLOS_UX_SECURITY_BOOT_DELAY
            || G_ux_params.ux_id == BOLOS_UX_BOOT_MENU
            || G_ux_params.ux_id == BOLOS_UX_CONSENT_NOT_INTERACTIVE_ONBOARD
          //|| G_ux_params.ux_id == BOLOS_UX_CONTROL_CENTER
          //|| G_ux_params.ux_id == BOLOS_UX_VALIDATE_PIN
          //|| G_ux_params.ux_id == BOLOS_UX_PAIRING_REQUEST
          //|| G_ux_params.ux_id == BOLOS_UX_PAIRING_CANCEL
          //END BOLOS MANAGER FLOW
          )) {
          ux_stack_push();
        }

        switch(G_ux_params.ux_id) {

          case BOLOS_UX_BOOT:
            // init seproxyhal ux related globals
            io_seproxyhal_init_ux();
            // no button push so far
            io_seproxyhal_init_button();
  #if 0
            // init the ram context
            os_memset(&G_bolos_ux_context, 0, sizeof(G_bolos_ux_context));
            // setup the ram canary
            G_bolos_ux_context.canary = CANARY_MAGIC;
            // register the ux parameters pointer for the os side
            os_ux_register(&G_ux_params);
            G_bolos_ux_context.state = STATE_INITIALIZED;
  #endif 

            // return, this should be the first and only call from the bolos task at platform startup
            G_ux.exit_code = BOLOS_UX_OK;
            
          case BOLOS_UX_BOLOS_START:

            screen_wake_up();
            // apply settings in the L4 (ble, brightness, etc)
            screen_settings_apply();

            // ensure ticker is present
            io_seproxyhal_setup_ticker(100);

            // diretly ask for a MCU status event to update the battery level
            io_seproxyhal_request_mcu_status();

            // no break is intentional
          default:
            // nothing to do yet
            G_ux.exit_code = BOLOS_UX_OK;
            break;

          case BOLOS_UX_ASYNCHMODAL_PAIRING_REQUEST:
            screen_wake_up();
            ux_pairing_init();
            break;

          case BOLOS_UX_ASYNCHMODAL_PAIRING_CANCEL:
            //screen_wake_up(); // it this really needed ?
            ux_pairing_deinit();
            break;

          case BOLOS_UX_BOOT_MENU:
            screen_wake_up();
            ux_boot_menu_init();
            break;

          case BOLOS_UX_MCU_UPGRADE_REQUIRED:
            screen_wake_up();
            screen_mcu_upgrade_required_init();
            break;

  #ifndef BOLOS_OS_UPGRADER
          case BOLOS_UX_PREPARE_RUN_APP:
            screen_dashboard_disable_bolos_before_app();
            G_ux.exit_code = BOLOS_UX_OK;
            break;
            
          case BOLOS_UX_BOOT_ONBOARDING:
            screen_wake_up();
            // re apply settings in the L4 (ble, brightness, etc) after exiting application in case of wipe
            screen_settings_apply();

            // enable control center from here
            G_bolos_ux_context.control_center_enabled = BOLOS_UX_OK;

    #ifdef BOLOS_FIXED_ONBOARDING
            os_fixed_onboarding();
    #endif // BOLOS_FIXED_ONBOARDING

            // avoid reperso is already onboarded to avoid leaking data through parameters due to user land call
            if (os_perso_isonboarded() == BOLOS_UX_OK) {
              G_ux.exit_code = BOLOS_UX_OK;
              break;
            }

            //screen_onboarding_0_welcome_init();
            ux_ob_onboarding_init();
            break;

          case BOLOS_UX_DASHBOARD:

  #ifdef BOLOS_NO_CONSENT
            if (os_perso_isonboarded() == BOLOS_UX_OK) {
              // upon no consent, default validate the base identity when reaching the dashboard
              os_global_pin_check(DEFAULT_PIN, 4);
            }
  #endif // BOLOS_NO_CONSENT
  #ifdef BOLOS_FIXED_ONBOARDING
            if (os_perso_isonboarded() != BOLOS_UX_OK) {
              os_fixed_onboarding();
            }
  #endif // BOLOS_FIXED_ONBOARDING

            screen_wake_up();

            // apply settings when redisplaying dashboard
            screen_settings_apply();

            // enable control center from here
            G_bolos_ux_context.control_center_enabled = BOLOS_UX_OK;
        
            // when returning from application, the ticker could have been disabled
            io_seproxyhal_setup_ticker(100);

  #ifdef BOLOS_AUTOSTART_FIRST
            // if not in recovery
            if (
#ifndef BOLOS_FORCE_RECOVERY
              !(os_flags() & OS_FLAG_RECOVERY) && 
#endif // BOLOS_FORCE_RECOVERY
              !G_bolos_ux_context.app_auto_started) {
              // run the first non ux application
              unsigned int i = 0;
              while (i < os_registry_count()) {
                application_t app;
                os_registry_get(i, &app);
                if (!(app.flags & (APPLICATION_FLAG_BOLOS_UX|APPLICATION_FLAG_NO_RUN))) {
                  G_bolos_ux_context.app_auto_started = 1;

                  ux_stack_pop();
                  io_seproxyhal_disable_io();

                  os_sched_exec(i); // no return
                }
              }
            }
  #else
            // if not in recovery, and app not yet started since power on (to avoid auto relaunch when exiting the app voluntarily)
            if (
// allow for running default app on test boards.
#ifndef BOLOS_FORCE_RECOVERY
              !(os_flags() & OS_FLAG_RECOVERY) && 
#endif // BOLOS_FORCE_RECOVERY
              !G_bolos_ux_context.app_auto_started) {
              // run the first non ux application that has the AUTOBOOT flag
              // start with the last app slot
              unsigned int i = os_registry_count();
              while (i--) {
                application_t app;
                os_registry_get(i, &app);
                if (!(app.flags & (APPLICATION_FLAG_BOLOS_UX|APPLICATION_FLAG_NO_RUN)) 
                  && (app.flags & APPLICATION_FLAG_AUTOBOOT)
                  // only trusted apps can autoboot
                  && (app.flags & (APPLICATION_FLAG_ISSUER|APPLICATION_FLAG_CUSTOM_CA|APPLICATION_FLAG_SIGNED))) {
                  G_bolos_ux_context.app_auto_started = 1;

                  // drop all ux overlays
                  ux_stack_pop();
                  // kill os ios
                  io_seproxyhal_disable_io();

                  // run the app
                  os_sched_exec(i);
                }
              }
            }
  #endif // BOLOS_AUTOSTART_FIRST
            // display a message to inform no app installed and that the user must go to ledgerwallet to start installing apps.
            screen_dashboard_init();
            break;

          case BOLOS_UX_KEYBOARD:
            screen_wake_up();
            // init with user buffer (to allow for chunking, use )
            os_memmove(G_bolos_ux_context.keyboard_user_buffer, G_ux_params.u.keyboard.entered_text, sizeof(G_bolos_ux_context.keyboard_user_buffer));
            // ensure a final EOS is present
            screen_keyboard_init((char*)G_bolos_ux_context.keyboard_user_buffer, 
                                 sizeof(G_bolos_ux_context.keyboard_user_buffer)-1, 
                                 screen_keyboard_validate_entered_text);
            break;

          case BOLOS_UX_VALIDATE_PIN:
            screen_wake_up();
            // enable control center to display its menu when pin is required at boot, to allow for power off
            G_bolos_ux_context.control_center_enabled = BOLOS_UX_OK;
            screen_modal_validate_pin_init();
            break;

          case BOLOS_UX_CONSENT_APP_ADD:
            screen_wake_up();
            screen_consent_add_init();
            break;

          case BOLOS_UX_CONSENT_APP_DEL:
            screen_wake_up();
            screen_consent_del_init();
            break;

          case BOLOS_UX_CONSENT_ISSUER_KEY:
            screen_wake_up();
            screen_consent_issuer_key_init();
            break;

          case BOLOS_UX_CONSENT_CUSTOMCA_KEY:
            screen_wake_up();
            screen_consent_customca_key_init();
            break;

          case BOLOS_UX_CONSENT_FOREIGN_KEY:
            screen_wake_up();
            screen_consent_foreign_key_init();
            break;

          case BOLOS_UX_CONSENT_GET_DEVICE_NAME:
            screen_wake_up();
            screen_consent_get_device_name_init();
            break;

          case BOLOS_UX_CONSENT_SET_DEVICE_NAME:
            screen_wake_up();
            screen_consent_set_device_name_init();
            break;

          case BOLOS_UX_BOOT_UX_NOT_SIGNED:
            screen_wake_up();
            screen_consent_ux_not_signed_init();
            break;

          case BOLOS_UX_BOOT_NOT_PERSONALIZED:
            screen_wake_up();
            screen_not_personalized_init();
            break;

          case BOLOS_UX_CONSENT_SETUP_CUSTOMCA_KEY:
            screen_wake_up();
            screen_consent_setup_customca_init();
            break;

          case BOLOS_UX_CONSENT_RESET_CUSTOMCA_KEY:
            screen_wake_up();
            screen_consent_reset_customca_init();
            break;

          case BOLOS_UX_CONSENT_RUN_APP:
            screen_wake_up();
            screen_consent_run_app_init();
            break;

          case BOLOS_UX_CONSENT_NOT_INTERACTIVE_ONBOARD:
            screen_wake_up();
            screen_consent_nio_init();
            break;

  #else // ! BOLOS_OS_UPGRADER
          // upgrader dashboard does not exists
          case BOLOS_UX_DASHBOARD:
            // enable control center from here
            G_bolos_ux_context.control_center_enabled = BOLOS_UX_OK;
            screen_wake_up();
            screen_os_upgrader();
            break;
  #endif // ! BOLOS_OS_UPGRADER

          case BOLOS_UX_SECURITY_BOOT_DELAY:
            screen_wake_up();
            screen_boot_delay_init();
            break;          

          // only consent upgrade is common to os upgrader and normal os to avoid being stuck if hash doesn't match
          case BOLOS_UX_CONSENT_UPGRADE:
            screen_wake_up();
            screen_consent_upgrade_init();
            break;
        
          // display a wait screen during application loading
          // if host computer bugs, then the token also remains in a loading state (on screen only)
          case BOLOS_UX_PROCESSING:
            screen_wake_up();
            screen_processing_init();
            break;

          case BOLOS_UX_WAKE_UP:  
            screen_wake_up();
            // if a screen is drawn (like the PIN) onto the current screen, then avoid allowing the app to erase or whatever the current screen
            break;

          // continue processing of the current screen
          case BOLOS_UX_EVENT: {
            // retrieve the last message received by the application, cached by the OS (to avoid complex and sluggish parameter copy interface in syscall)
            if (io_seproxyhal_spi_recv(G_io_seproxyhal_spi_buffer, sizeof(G_io_seproxyhal_spi_buffer), IO_CACHE) == 0) {
              // nothing received (maybe cache due to other task yielding)
              break;
            }

            // process event
            // nothing done with the event, throw an error on the transport layer if needed

            // just reply "amen"
            // add a "pairing ok" tag if necessary
            // can't have more than one tag in the reply, not supported yet.
            switch (G_io_seproxyhal_spi_buffer[0]) {

              case SEPROXYHAL_TAG_TICKER_EVENT: {
                unsigned int last_ms = G_bolos_ux_context.ms;
                unsigned int interval_ms = 0;
                if (G_io_seproxyhal_spi_buffer[2] == 4) {
                  G_bolos_ux_context.ms = U4BE(G_io_seproxyhal_spi_buffer, 3);
                }
                else {
                  G_bolos_ux_context.ms += 100; // ~ approx, just to avoid being stuck on blue dev edition 
                }

                // compute time interval, handle overflow
                interval_ms = G_bolos_ux_context.ms-last_ms;
                if (G_bolos_ux_context.ms < last_ms) {
                  interval_ms = (-1UL)-interval_ms;
                }

                if (G_bolos_ux_context.ms_last_batt_check + BATTERY_CHECK_INTERVAL_MS < G_bolos_ux_context.ms
                  // ask for current state at startup
                  || G_bolos_ux_context.ms_last_batt_check == 0) {
                  G_bolos_ux_context.ms_last_batt_check = G_bolos_ux_context.ms;
                  io_seproxyhal_request_mcu_status();
                }

                // log_debug_int_nw("ts %x ", G_bolos_ux_context.ms);

                // request time extension of the MCU watchdog
                G_io_seproxyhal_spi_buffer[0] = SEPROXYHAL_TAG_MORE_TIME;
                G_io_seproxyhal_spi_buffer[1] = 0;
                G_io_seproxyhal_spi_buffer[2] = 0;
                io_seproxyhal_spi_send(G_io_seproxyhal_spi_buffer, 3);

                if (G_ux.stack_count > 0 
                  && G_ux.stack[G_ux.stack_count-1].ticker_callback 
                  && G_ux.stack[G_ux.stack_count-1].ticker_interval) {
                  G_ux.stack[G_ux.stack_count-1].ticker_value -= MIN(G_ux.stack[G_ux.stack_count-1].ticker_value, interval_ms);
                  if (G_ux.stack[G_ux.stack_count-1].ticker_value == 0) {
                    // rearm, and call the registered function
                    G_ux.stack[G_ux.stack_count-1].ticker_value = G_ux.stack[G_ux.stack_count-1].ticker_interval;
                    G_ux.stack[G_ux.stack_count-1].ticker_callback(/*ignored*/ 0);
                  }
                }

                if (G_bolos_ux_context.ms_last_activity == 0) {
                  // initializing with no user action (at boot time, the user just ... wait)
                  G_bolos_ux_context.ms_last_activity = G_bolos_ux_context.ms;
                  G_bolos_ux_context.ms_last_activity_saver = G_bolos_ux_context.ms;
                  G_bolos_ux_context.ms_last_activity_pwroff = G_bolos_ux_context.ms;
                  G_bolos_ux_context.ms_last_batt_check = G_bolos_ux_context.ms;
                }
  // don't lock in os upgrade or when no consent mode
  #ifndef BOLOS_OS_UPGRADER
  #ifndef BOLOS_NO_SCREENLOCK
                else {
                  // screen lock
                  if (os_perso_isonboarded() == BOLOS_UX_OK 
                    && IS_SETTING_PRE_POWER_OFF()
                    && G_bolos_ux_context.inactivity_state < INACTIVITY_LOCK
                    && G_bolos_ux_context.ms > G_bolos_ux_context.ms_last_activity + INACTIVITY_MS_AUTO_LOCK) {
                    screen_lock();
                  }

                  // screen saver
                  if (IS_SETTING_PRE_POWER_OFF()
                    && G_bolos_ux_context.inactivity_state < INACTIVITY_SAVER
                    && G_bolos_ux_context.ms > G_bolos_ux_context.ms_last_activity_saver + INACTIVITY_MS_AUTO_LOCK) {
                    screen_lock(); // relock just in case
                    screen_save(); // run the screen saver (could have been popped by a wakeup call)
                  }

                  // power off
                  if (IS_SETTING_POWER_OFF()
                    // only power off when not charging or not USB powered
                    && ! (G_bolos_ux_context.status_flags &(SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED|SEPROXYHAL_TAG_STATUS_EVENT_FLAG_CHARGING) )
                    //&& G_bolos_ux_context.inactivity_state < INACTIVITY_DEVICE_OFF 
                    && G_bolos_ux_context.ms > G_bolos_ux_context.ms_last_activity_pwroff + INACTIVITY_MS_POWER_OFF) {
                    //G_bolos_ux_context.inactivity_state = INACTIVITY_DEVICE_OFF;
                    io_seproxyhal_power_off(); // NO RETURN HERE
                  }
                }
  #endif // BOLOS_NO_SCREENLOCK
  #endif // BOLOS_OS_UPGRADER
                // user manual screen lock, by pressing both buttons for a long period of time
                if (G_bolos_ux_context.control_center_enabled == BOLOS_UX_OK
                  && G_bolos_ux_context.inactivity_state != INACTIVITY_SAVER) {

                  // one button has been release ? 
                  switch(G_bolos_ux_context.last_button_state) {
                    case (BUTTON_LEFT|BUTTON_RIGHT):
                      // if not yet triggered, then startup the both button sequence check
                      if (G_bolos_ux_context.ms_cc_start == 0) {
                        G_bolos_ux_context.ms_cc_start = G_bolos_ux_context.ms;
                      }
                      else {
                        if (G_bolos_ux_context.ms >= G_bolos_ux_context.ms_cc_start + BOLOS_CC_PERIOD_MS ) {
                          // when not onboarded, can display the control center nevertheless
                          //if (screen_is_not_locked())  // always possible to power off
                          {
                            screen_control_center_init(1);
                            // we're displaying the control center
                            G_bolos_ux_context.control_center_entering = 1;
                            G_bolos_ux_context.ms_cc_start = 0;
                          }
                        }
                      }
                      break;
                    default:
                      // reset both button press for control center as the delay to have it on screen has not been exceeded
                      G_bolos_ux_context.ms_cc_start = 0;
                      break;
                  }
                }
                break;
              }

              // power off if long push, else pass to the application callback if any
              case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: {
                // user activity detected
                G_bolos_ux_context.ms_last_activity = G_bolos_ux_context.ms;
                G_bolos_ux_context.ms_last_activity_saver = G_bolos_ux_context.ms;
                G_bolos_ux_context.ms_last_activity_pwroff = G_bolos_ux_context.ms;
                G_bolos_ux_context.last_button_state = G_io_seproxyhal_spi_buffer[3]>>1;

                // log_debug_int_nw("btnux %x ", G_bolos_ux_context.last_button_state);

                // if any screen slot, then serve it
                if (G_ux.stack_count) {
                  // avoid the last button release of the both button hold that triggered the control center
                  // display to be processed by the control center without a prior button press state
                  if (! G_bolos_ux_context.control_center_entering) {
                    // will use the exit code from the currently displayed screen
                    io_seproxyhal_button_push(G_ux.stack[G_ux.stack_count-1].button_push_callback, 
                                              G_bolos_ux_context.last_button_state);
                  }
                  else {
                    // wake up when pushing button to enter the control center if was in saver, just to notice the action
                    screen_wake_up();
                  }
                }

                // reset the control center launch state to reallow buttons to be interpreted again 
                if (G_bolos_ux_context.control_center_entering) {
                  //G_bolos_ux_context.inactivity_state = INACTIVITY_NONE; // could still be locked when triggering the control center
                  G_bolos_ux_context.control_center_entering = 0;
                  io_seproxyhal_init_button();
                }
                break;
              }

              case SEPROXYHAL_TAG_STATUS_EVENT: {
                unsigned int flags = U4BE(G_io_seproxyhal_spi_buffer, 3);
                //unsigned int backlight = U4BE(G_io_seproxyhal_spi_buffer, 3+4);
                //unsigned int ledcolor = U4BE(G_io_seproxyhal_spi_buffer, 3+4+1);
                unsigned int battlevel = U4BE(G_io_seproxyhal_spi_buffer, 3+4+1+4);
                unsigned int battpercent = G_io_seproxyhal_spi_buffer[3+4+1+4+4];
                unsigned int battcurrent = G_io_seproxyhal_spi_buffer[3+4+1+4+4+1];

                // usb changed 
                if ((G_bolos_ux_context.status_flags & SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED) 
                  ^ (flags & SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
                  // usb plugged in
                  if ((flags & SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
                    // propose the pin prompt
                    screen_wake_up();
                  }
                  // usb plugged out
                  else {
                    // reset the power off delay to avoid immediate power off.
                    G_bolos_ux_context.ms_last_activity_pwroff = G_bolos_ux_context.ms;
                  }
                }

                // status bar change requested upon detected change
                if (G_bolos_ux_context.status_flags != flags 
                  || G_bolos_ux_context.status_batt_level != battlevel
                  || G_bolos_ux_context.battery_percentage != battpercent) {
                  G_bolos_ux_context.overlay_refresh = 1;
                }

                // store data for the status bar
                G_bolos_ux_context.status_flags = flags;
                G_bolos_ux_context.status_batt_level = battlevel;
                G_bolos_ux_context.battery_percentage = battpercent;


#ifndef BOLOS_RELEASE
                SPRINTF(G_bolos_ux_context.control_center_string, "%d%% %d", G_bolos_ux_context.battery_percentage, G_bolos_ux_context.status_batt_level);
                log_debug_nw(G_bolos_ux_context.control_center_string);
#endif // BOLOS_RELEASE

                if (G_bolos_ux_context.overlay_refresh) {
                  G_bolos_ux_context.overlay_refresh = 0;
                  if (G_bolos_ux_context.inactivity_state < INACTIVITY_SAVER) {
                    screen_display_overlay();
                  }
                }

                // redisplay the control center, when the battery state changes
                extern const bagl_element_t screen_control_center_elements[];
                if (G_ux.stack[G_ux.stack_count-1].element_arrays[0].element_array == screen_control_center_elements) {
                  ux_stack_redisplay();
                }

                // charging or USB powered implies no warning, even if draining faster than charging
                if (! (flags & (SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED))
                  // don't do this until a REAL percentage value is available
                  && G_bolos_ux_context.battery_percentage != 0xFFu) {
                  // wow, much low !
                  if (battpercent < BATTERY_AUTO_POWER_OFF_LEVEL_PERCENT) {
                    G_bolos_ux_context.batt_critical_displayed = 1;
                    G_bolos_ux_context.batt_low_displayed = 1;
#ifndef BOLOS_RELEASE
                    log_debug(G_bolos_ux_context.control_center_string);
#endif // BOLOS_RELEASE
                    io_seproxyhal_power_off();
                  }
                  else if (!G_bolos_ux_context.batt_critical_displayed 
                    && battpercent <= BATTERY_CRITICAL_LEVEL_PERCENT) {
                    G_bolos_ux_context.batt_critical_displayed = 1;
                    G_bolos_ux_context.batt_low_displayed = 1; // consumes low if not seen
                    screen_modal_crit_batt_init();
                    break;
                  }
                  else if (!G_bolos_ux_context.batt_low_displayed 
                    && battpercent <= BATTERY_LOW_LEVEL_PERCENT) {
                    G_bolos_ux_context.batt_low_displayed = 1;
                    screen_modal_low_batt_init();
                    break;
                  }
                }
                else {
                  // allow to repop battery alerts when battery has reach a minimum charge.
                  if (battpercent >= BATTERY_SUFFICIENT_CHARGE_PERCENT) {
                    G_bolos_ux_context.batt_low_displayed = 0;
                    G_bolos_ux_context.batt_critical_displayed = 0;
                  }
                }
                break;
              }
            } // switch evt id
            break;
          } // case UX_EVENT
        }
        break;
    }

    // remember the last displayed screen for blanking
    if (G_ux_params.ux_id != BOLOS_UX_EVENT) {
      G_bolos_ux_context.last_ux_id = G_ux_params.ux_id;
    }

    // kthx, but no
    if (G_bolos_ux_context.canary != CANARY_MAGIC) {
      halt();
    }

    if (G_ux.stack_count == 0) {
      // if a screen has been popped, then ask app redraw
      if (previous_screen_stack_count) {
        G_ux.exit_code = BOLOS_UX_REDRAW;
      }
      else
      {
        // nothing to display, it lgooks like we're not going to overlay the app anymore
        G_ux.exit_code = BOLOS_UX_OK;
      }
    }

    /* feilz during BOOT_UX in the boot sequence. at least many ux calls are not bound to user interaction even if not unlocked
    // can't possibly et the app continue if we're locked
    if (!screen_is_not_locked()) {
      if (G_ux.exit_code != BOLOS_UX_CONTINUE
        && G_ux.exit_code != BOLOS_UX_IGNORE) {
        G_ux.exit_code = BOLOS_UX_CONTINUE;
      }
    }
    */

    //BEGIN INVARIANT
    /*
    // invariant, can't return to application with an ending exit code when locked.
    if (G_ux.exit_code != BOLOS_UX_CONTINUE && G_bolos_ux_context.inactivity_state != INACTIVITY_NONE) {
      reset();
    }
    */

    /*
    extern const bagl_element_t screen_modal_validate_pin_end[1];
    if (ux_stack_is_element_array_present((const bagl_element_t*)screen_modal_validate_pin_end) && G_bolos_ux_context.inactivity_state == INACTIVITY_NONE) {
      reset();
    }
    */

    //END INVARIANT
    // 
  }
  CATCH_ALL {
    G_ux.exit_code = BOLOS_UX_CONTINUE;
  }
  FINALLY {
    
  }
}
END_TRY;
   // avoid reprocessing if bolos ux is re-run due to a yield
    G_ux_params.ux_id = BOLOS_UX_LAST_ID;
    // give up and wait to be awakened again
    os_sched_yield(G_ux.exit_code);
  }
  // return to the caller
  os_sched_exit(G_ux.exit_code);
}

void bolos_ux_hslider3_init(unsigned int total_count) {
  G_bolos_ux_context.hslider3_total = total_count; 
  switch (total_count) {
    case 0:
      G_bolos_ux_context.hslider3_before = BOLOS_UX_HSLIDER3_NONE;
      G_bolos_ux_context.hslider3_current = BOLOS_UX_HSLIDER3_NONE;
      G_bolos_ux_context.hslider3_after = BOLOS_UX_HSLIDER3_NONE;
      break;
    case 1:
      G_bolos_ux_context.hslider3_before = BOLOS_UX_HSLIDER3_NONE;
      G_bolos_ux_context.hslider3_current = 0;
      G_bolos_ux_context.hslider3_after = BOLOS_UX_HSLIDER3_NONE;
      break;
    case 2:
      G_bolos_ux_context.hslider3_before = BOLOS_UX_HSLIDER3_NONE;
      //G_bolos_ux_context.hslider3_before = 1; // full rotate
      G_bolos_ux_context.hslider3_current = 0;
      G_bolos_ux_context.hslider3_after = 1;
      break;
    default:
      G_bolos_ux_context.hslider3_before = total_count-1;
      G_bolos_ux_context.hslider3_current = 0;
      G_bolos_ux_context.hslider3_after = 1;
      break;
  }
}

void bolos_ux_hslider3_set_current(unsigned int current) {
  // index is reachable ?
  if (G_bolos_ux_context.hslider3_total > current) {
    // reach it
    while (G_bolos_ux_context.hslider3_current != current) {
      bolos_ux_hslider3_next();
    }
  }
}

#if 0
/*
void bolos_ux_hslider3_next(void) {
  switch (G_bolos_ux_context.hslider3_total) {
    case 0:
    case 1:
      break;
    case 2:
      if (G_bolos_ux_context.hslider3_before == 0) {
        // simulate a previous, as we're at the beginning
        G_bolos_ux_context.hslider3_before = BOLOS_UX_HSLIDER3_NONE;
        G_bolos_ux_context.hslider3_current = 0;
        G_bolos_ux_context.hslider3_after = 1;        
      }
      else {
        G_bolos_ux_context.hslider3_before = 0;
        G_bolos_ux_context.hslider3_current = 1;
        G_bolos_ux_context.hslider3_after = BOLOS_UX_HSLIDER3_NONE;
      }
      break;
    default:
      G_bolos_ux_context.hslider3_before = G_bolos_ux_context.hslider3_current;
      G_bolos_ux_context.hslider3_current = G_bolos_ux_context.hslider3_after;
      G_bolos_ux_context.hslider3_after = (G_bolos_ux_context.hslider3_after+1)%G_bolos_ux_context.hslider3_total;
      break;
  }
}

void bolos_ux_hslider3_previous(void) {
  switch (G_bolos_ux_context.hslider3_total) {
    case 0:
    case 1:
      break;
    case 2:
      if (G_bolos_ux_context.hslider3_after == 1) {
        // simulate a next, as we're at the end
        G_bolos_ux_context.hslider3_before = 0;
        G_bolos_ux_context.hslider3_current = 1;
        G_bolos_ux_context.hslider3_after = BOLOS_UX_HSLIDER3_NONE;
      }
      else {
        G_bolos_ux_context.hslider3_before = BOLOS_UX_HSLIDER3_NONE;
        G_bolos_ux_context.hslider3_current = 0;
        G_bolos_ux_context.hslider3_after = 1;
      }
      break;
    default:
      G_bolos_ux_context.hslider3_after = G_bolos_ux_context.hslider3_current;
      G_bolos_ux_context.hslider3_current = G_bolos_ux_context.hslider3_before;
      G_bolos_ux_context.hslider3_before = (G_bolos_ux_context.hslider3_before+G_bolos_ux_context.hslider3_total-1)%G_bolos_ux_context.hslider3_total;
      break;
  }
}
*/
#endif

void bolos_ux_hslider3_next(void) {
  switch (G_bolos_ux_context.hslider3_total) {
    case 0:
    case 1:
      break;
    case 2:
      switch(G_bolos_ux_context.hslider3_current) {
        case 0:
          G_bolos_ux_context.hslider3_before = 0;  
          G_bolos_ux_context.hslider3_current = 1;
          G_bolos_ux_context.hslider3_after = BOLOS_UX_HSLIDER3_NONE;   
          break;
        case 1:
          G_bolos_ux_context.hslider3_before = BOLOS_UX_HSLIDER3_NONE;
          G_bolos_ux_context.hslider3_current = 0;
          G_bolos_ux_context.hslider3_after = 1;        
          break;
      }
      break;
    default:
      G_bolos_ux_context.hslider3_before = G_bolos_ux_context.hslider3_current;
      G_bolos_ux_context.hslider3_current = G_bolos_ux_context.hslider3_after;
      G_bolos_ux_context.hslider3_after = (G_bolos_ux_context.hslider3_after+1)%G_bolos_ux_context.hslider3_total;
      break;
  }
}

void bolos_ux_hslider3_previous(void) {
  switch (G_bolos_ux_context.hslider3_total) {
    case 0:
    case 1:
      break;
    case 2:
      switch(G_bolos_ux_context.hslider3_current) {
        case 0:
          G_bolos_ux_context.hslider3_before = 0;  
          G_bolos_ux_context.hslider3_current = 1;
          G_bolos_ux_context.hslider3_after = BOLOS_UX_HSLIDER3_NONE;   
          break;
        case 1:
          G_bolos_ux_context.hslider3_before = BOLOS_UX_HSLIDER3_NONE;
          G_bolos_ux_context.hslider3_current = 0;
          G_bolos_ux_context.hslider3_after = 1;        
          break;
      }
      break;
    default:
      G_bolos_ux_context.hslider3_after = G_bolos_ux_context.hslider3_current;
      G_bolos_ux_context.hslider3_current = G_bolos_ux_context.hslider3_before;
      G_bolos_ux_context.hslider3_before = (G_bolos_ux_context.hslider3_before+G_bolos_ux_context.hslider3_total-1)%G_bolos_ux_context.hslider3_total;
      break;
  }
}

unsigned int screen_consent_button(unsigned int button_mask, unsigned int button_mask_counter) {
  UNUSED(button_mask_counter);
  switch(button_mask) {
    case BUTTON_EVT_RELEASED|BUTTON_LEFT:
      G_ux.exit_code = BOLOS_UX_CANCEL;
      break;
    case BUTTON_EVT_RELEASED|BUTTON_RIGHT:
      G_ux.exit_code = BOLOS_UX_OK;
      break;
  }
  return 0;
}

/*
enforced at bolos task level
unsigned int screen_consent_button_with_final_pin(unsigned int button_mask, unsigned int button_mask_counter) {
  UNUSED(button_mask_counter);
  switch(button_mask) {
    case BUTTON_EVT_RELEASED|BUTTON_LEFT:
      G_ux.exit_code = BOLOS_UX_CANCEL;
      break;
    case BUTTON_EVT_RELEASED|BUTTON_RIGHT:
      // ensure the PIN is the only stack layer to returns its exit code when validated
      ux_stack_pop();

      // ensure to ask for pin validation before continuing with upgrade to ensure seed won't be at risk
      screen_modal_validate_pin_init();
      break;
  }
  return 0;
}
*/

unsigned int screen_consent_ticker(unsigned int ignored) {
  UNUSED(ignored);

  // prepare displaying next screen
  G_bolos_ux_context.onboarding_index=(G_bolos_ux_context.onboarding_index+1)%G_bolos_ux_context.onboarding_step;

  // redisplay taking into account the new counter
  ux_stack_display(0);
  return 0;
}

void screen_consent_set_interval(unsigned int interval_ms) {
  G_ux.stack[0].ticker_value = interval_ms;
  G_ux.stack[0].ticker_interval = interval_ms; 
}

void screen_consent_ticker_init(unsigned int number_of_steps, unsigned int interval_ms, unsigned int check_pin_to_confirm) {
  UNUSED(check_pin_to_confirm);
  // register action callbacks
  G_ux.stack[0].ticker_value = interval_ms;
  G_ux.stack[0].ticker_interval = interval_ms; 
  G_ux.stack[0].ticker_callback = screen_consent_ticker;
  /*if (!check_pin_to_confirm || ! os_perso_isonboarded() == BOLOS_UX_OK) { managed at bolos task level */
    G_ux.stack[0].button_push_callback = screen_consent_button;
  /*}
  else {
    G_ux.stack[0].button_push_callback = screen_consent_button_with_final_pin;
  }*/

  // start displaying
  G_bolos_ux_context.onboarding_index = number_of_steps-1;
  G_bolos_ux_context.onboarding_step  = number_of_steps;
  screen_consent_ticker(0);
}

#endif // OS_IO_SEPROXYHAL

#endif
