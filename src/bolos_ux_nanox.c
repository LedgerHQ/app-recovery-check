#include "os.h"

#ifdef TARGET_NANOX

#include "cx.h"
#include "ux.h"

#include "os_io_seproxyhal.h"
#include "string.h"

#include "bolos_ux_common.h"
#include "glyphs.h"

#ifdef OS_IO_SEPROXYHAL

bolos_ux_context_t G_bolos_ux_context;



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

