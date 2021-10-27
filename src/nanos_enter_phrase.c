/*******************************************************************************
*   Ledger Blue - Secure firmware
*   (c) 2016, 2017 Ledger
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "ui.h"
#include "bolos_ux_common.h"

#ifdef TARGET_NANOS


// allow to edit back any entered word
#define RESTORE_WORD_MAX_BACKWARD_STEPS 24


const bagl_element_t* screen_onboarding_4_restore_word_before_element_display_callback(const bagl_element_t* element);

void screen_onboarding_4_restore_word_display_auto_complete(void);
UX_STEP_CB(
  restore_4_intro_1,
  nn,
  screen_onboarding_4_restore_word_display_auto_complete();,
  {
    "Enter", 
    G_ux.string_buffer
  });

UX_FLOW(
  restore_4_intro,
  &restore_4_intro_1
  );

UX_STEP_CB(
  restore_4_invalid_1,
  pbb,
  screen_onboarding_3_restore_init();,
  {
    &C_icon_warning,
    "Recovery", 
    "phrase invalid"
  });

UX_FLOW(
  restore_4_invalid,
  &restore_4_invalid_1);

UX_STEP_NOCB(
    ux_nomatch_step_1,
    pbb,
    {
      &C_icon_warning,
      "Phrase",
      "doesn't match"
    });
UX_STEP_NOCB(
    ux_nomatch_step_2,
    nn,
    {
      "Check length",
      "order and spelling",
    });
UX_STEP_VALID(
    ux_nomatch_step_3,
    pb,
    ui_idle_init(),
    {
      &C_icon_back_x,
      "Return to menu"
    });

UX_FLOW(
  flow_final_nomatch,
  &ux_nomatch_step_1,
  &ux_nomatch_step_2,
  &ux_nomatch_step_3
);

UX_STEP_CB(
    step_final_match,
    pbb,
    os_sched_exit(0);,
    {
        &C_icon_validate_14,
        "Phrase",
        "is correct"
    });

UX_FLOW(flow_final_match, &step_final_match);


void screen_processing_postinit(unsigned int stack_slot) {
  // ensure when pin is modal over the processing screen( at end of install) the processing screen gives back the hand to the calling code
  G_ux.stack[stack_slot].exit_code_after_elements_displayed = BOLOS_UX_OK;
}

UX_STEP_NOCB_POSTINIT(
  processing_step,
  pb,
  screen_processing_postinit(stack_slot),
  {
    &C_icon_loader,
    "Processing",
  });

UX_FLOW(processing_flow,
  &processing_step);

void screen_processing_init(void) {
  ux_flow_init(0, processing_flow, NULL);
}

unsigned int screen_onboarding_4_restore_word_select_button(unsigned int button_mask, unsigned int button_mask_counter);

#define ITEMS (G_ux.string_buffer+32)

const bagl_element_t* screen_onboarding_4_restore_word_keyboard_callback(unsigned int event, unsigned int value);

void screen_onboarding_4_restore_word_display_auto_complete(void) {
  unsigned int auto_complete_count = bolos_ux_bip39_get_word_next_letters_starting_with((unsigned char*)G_ux.string_buffer+16, 
                                                                                 strlen(G_ux.string_buffer+16), 
                                                                                 (unsigned char*)G_ux.string_buffer+32);
    // display added letter and refresh slider
  screen_common_keyboard_init(0, 
                              0, /*always restart from the first element in the list*/ 
                              //(strlen(G_ux.string_buffer+16)?0:cx_rng_u8()%auto_complete_count), /* start from a random element in the list for the word start letter, else keep the order */
        // recompute alphabet and set the number of elements in the keyboard
                              auto_complete_count
                              + (strlen(G_ux.string_buffer+16)?1:0) /* backspace if already a stem enetered, else no backspace */, 
        screen_onboarding_4_restore_word_keyboard_callback);
  // append the special backspace to allow for easier dispatch in the keyboard callback
  ((unsigned char*)(G_ux.string_buffer+32))[auto_complete_count] = '\b'; 
}

void screen_onboarding_4_restore_word_display_word_selection(void) {
  ux_stack_init(0);
  G_ux.stack[0].button_push_callback = screen_onboarding_4_restore_word_select_button;
  G_ux.stack[0].element_arrays[0].element_array = screen_onboarding_word_list_elements;
  G_ux.stack[0].element_arrays[0].element_array_count = sizeof(screen_onboarding_word_list_elements)/sizeof(screen_onboarding_word_list_elements[0]);
  G_ux.stack[0].element_arrays_count = 1;
  G_ux.stack[0].screen_before_element_display_callback = screen_onboarding_4_restore_word_before_element_display_callback;
  ux_stack_display(0); 
}

const bagl_element_t* screen_onboarding_4_restore_word_keyboard_callback(unsigned int event, unsigned int value) {
  switch(event) {
    case KEYBOARD_ITEM_VALIDATED:
        // depending on the chosen class, interpret the click
      if (G_ux.string_buffer[32+value] == '\b') {
        if (strlen(G_ux.string_buffer+16)) {
          G_ux.string_buffer[16+strlen(G_ux.string_buffer+16)-1] = 0;
                screen_onboarding_4_restore_word_display_auto_complete();
            }
        }
      else {
            // validate next letter of the word
        G_ux.string_buffer[16+strlen(G_ux.string_buffer+16)] = G_ux.string_buffer[32+G_bolos_ux_context.hslider3_current];

            // continue displaying until less than X words matches the stem
        G_bolos_ux_context.onboarding_words_checked = bolos_ux_bip39_get_word_count_starting_with((unsigned char*)G_ux.string_buffer+16, strlen(G_ux.string_buffer+16));
        if (G_bolos_ux_context.onboarding_words_checked > ONBOARDING_WORD_COMPLETION_MAX_ITEMS) {
          // too much words for slider word completion, await another letter
                screen_onboarding_4_restore_word_display_auto_complete();
            }
        else {
                // always init stem count
                // index of the first word matching the stem
          G_bolos_ux_context.onboarding_index = bolos_ux_bip39_get_word_idx_starting_with((unsigned char*)G_ux.string_buffer+16, strlen(G_ux.string_buffer+16));

                // multiple possibilities
                // update te slider's possible words
          // account for the extra clear word, and clear any previous word items (go back in the onboarding process)
          bolos_ux_hslider3_init(G_bolos_ux_context.onboarding_words_checked+MIN(G_bolos_ux_context.onboarding_step+1, RESTORE_WORD_MAX_BACKWARD_STEPS));
                screen_onboarding_4_restore_word_display_word_selection();
            }
            return NULL;
        }
        break;

    case KEYBOARD_RENDER_ITEM:
      if (G_ux.string_buffer[32+value] == '\b') {
            /*
            x:57 w:14
            iconw: 11
            */
        G_ux.tmp_element.component.width = C_icon_backspace.width;
        G_ux.tmp_element.component.x += 1 + G_ux.tmp_element.component.width/2 - C_icon_backspace.width/2;
        G_ux.tmp_element.component.y = 5;
        G_ux.tmp_element.component.height = C_icon_backspace.height;
        G_ux.tmp_element.component.type = BAGL_ICON;
        G_ux.tmp_element.component.icon_id = 0;
        // if current selected, then display the inversed digit (as in a pin digit)
        G_ux.tmp_element.text = (const char*)&C_icon_backspace;
        }
      else {
        //G_ux.string_buffer[0] = G_ux.string_buffer[32+value]-'a'+'A'; // render as uppercase, always
        G_ux.string_buffer[0] = G_ux.string_buffer[32+value]; // render as lowercase, always
        G_ux.string_buffer[1] = 0;
        G_ux.tmp_element.text = G_ux.string_buffer;
        }
        break;
    case KEYBOARD_RENDER_WORD: {
      unsigned int l = strlen(G_ux.string_buffer+16);

      if (value < 8) {
        G_ux.tmp_element.component.x += 5;
        // prefix word stem with "%d:" with the current word index
        snprintf(G_ux.string_buffer+2, 5, "#%d ", G_bolos_ux_context.onboarding_step + 1);
            // ensure font is left aligned
        G_ux.tmp_element.text = G_ux.string_buffer;
        G_ux.string_buffer[1] = 0;
        if (value<strlen(G_ux.string_buffer+2)) {
          G_ux.string_buffer[0] = G_ux.string_buffer[2+value];
                }
        else if (value<l+strlen(G_ux.string_buffer+2)) {
          G_ux.string_buffer[0] = G_ux.string_buffer[16+value-strlen(G_ux.string_buffer+2)];
                }
        else {
          G_ux.string_buffer[0] = '_';
            }
        // will never occur on word stem, autocomplete always happen before that
        // else {
        //   // first char is '...' to notify continuing
        //   if (value == 0) {
        //     G_ux.string_buffer[0] = '.';
        //     G_ux.string_buffer[1] = '.';
        //     G_ux.string_buffer[2] = '.';
        //     G_ux.string_buffer[3] = 0;
        //   }
        //   else {
        //     G_ux.string_buffer[0] = (G_ux.string_buffer+16+l+1-8)[value];
        //   }
        // }
            break;
        }
        return NULL;
    }
    }
    // update element display
  return &G_ux.tmp_element;
}

void compare_recovery_phrase(void)
{
    G_bolos_ux_context.processing = 0;

    io_seproxyhal_general_status();

    // convert mnemonic to hex-seed
    uint8_t buffer[64];

    bolos_ux_mnemonic_to_seed((unsigned char *)G_bolos_ux_context.words_buffer, 
    G_bolos_ux_context.words_buffer_length,
    buffer);
    //PRINTF("Input seed:\n %.*H\n", 64, buffer);

    // get rootkey from hex-seed
    cx_hmac_sha512_t ctx;
    const char key[] = "Bitcoin seed";

    cx_hmac_sha512_init(&ctx, (const uint8_t *) key, sizeof(key));
    cx_hmac((cx_hmac_t *) &ctx,CX_LAST, buffer, 64, buffer, 64);
    //PRINTF("Root key from input:\n%.*H\n", 64, buffer);

    // get rootkey from device's seed
    uint8_t buffer_device[64];

    os_perso_derive_node_bip32(CX_CURVE_256K1, NULL, 0, buffer_device, buffer_device+32);
    //PRINTF("Root key from device: \n%.*H\n", 64, buffer_device);

    // compare both rootkey
    if(os_secure_memcmp(buffer, buffer_device, 64)) {
        ux_flow_init(0, flow_final_nomatch, NULL);
    }
    else {
        ux_flow_init(0, flow_final_match, NULL);
    }
}


const bagl_element_t* screen_onboarding_4_restore_word_before_element_display_callback(const bagl_element_t* element) {
  switch(element->component.userid) {

    case 0x01:
      if (G_bolos_ux_context.hslider3_total == 0 || G_bolos_ux_context.hslider3_current >= G_bolos_ux_context.onboarding_words_checked) {
        return NULL;
      }
      SPRINTF(G_ux.string_buffer, "Word #%d", G_bolos_ux_context.onboarding_step + 1);
      break;

    case 0x02:
      if (G_bolos_ux_context.hslider3_total == 0 || G_bolos_ux_context.hslider3_current >= G_bolos_ux_context.onboarding_words_checked) {
        return NULL;
      }
      // display matching word from the slider's current index
      bolos_ux_bip39_idx_strcpy(G_bolos_ux_context.onboarding_index + G_bolos_ux_context.hslider3_current, (unsigned char*)G_ux.string_buffer);
      break;

    case 0x03:
      // no left button on first item
      if (G_bolos_ux_context.hslider3_current == 0) {
        return NULL;
      }
      break;
    
    case 0x04:
      // no right button on last item
      if (G_bolos_ux_context.hslider3_total == 0 || G_bolos_ux_context.hslider3_current >= G_bolos_ux_context.hslider3_total-1) {
        return NULL;
      }
      break;

    case 0x05:
      if (G_bolos_ux_context.hslider3_total == 0 || G_bolos_ux_context.hslider3_current < G_bolos_ux_context.onboarding_words_checked) {
        return NULL;
      }
      break;
    case 0x06:
      if (G_bolos_ux_context.hslider3_total == 0 || G_bolos_ux_context.hslider3_current < G_bolos_ux_context.onboarding_words_checked) {
        return NULL;
      }
      SPRINTF(G_ux.string_buffer, "word #%d", G_bolos_ux_context.hslider3_total - G_bolos_ux_context.hslider3_current);
      break;
  }

  // display other elements only if screen setup, else, only redraw words value
  return element;
}


void screen_onboarding_4_restore_word_validate(void) {
  bolos_ux_bip39_idx_strcpy(G_bolos_ux_context.onboarding_index + G_bolos_ux_context.hslider3_current, (unsigned char*)(G_bolos_ux_context.words_buffer+G_bolos_ux_context.words_buffer_length));
  G_bolos_ux_context.words_buffer_length=strlen(G_bolos_ux_context.words_buffer);

    // a word has been added
    G_bolos_ux_context.onboarding_step++;

  if (G_bolos_ux_context.onboarding_step == G_bolos_ux_context.onboarding_kind) {
        unsigned int valid;

#ifdef HAVE_ELECTRUM    
    // if we've entered all the words, then check the phrase
    if (G_bolos_ux_context.onboarding_algorithm == BOLOS_UX_ONBOARDING_ALGORITHM_ELECTRUM) {
      valid = bolos_ux_electrum_mnemonic_check(ELECTRUM_SEED_PREFIX_STANDARD, (unsigned char*)G_bolos_ux_context.words_buffer, G_bolos_ux_context.words_buffer_length);
        }
    else {
      valid = bolos_ux_mnemonic_check((unsigned char*)G_bolos_ux_context.words_buffer, G_bolos_ux_context.words_buffer_length);

            
        }
#else
    valid = bolos_ux_mnemonic_check((unsigned char*)G_bolos_ux_context.words_buffer, G_bolos_ux_context.words_buffer_length);
#endif    
    if ( !valid  ) {
      ux_flow_init(0, restore_4_invalid, NULL);
    }
    else {
      // allright, the recovery phrase looks ok, finish onboarding
            //Display processing warning to user
            screen_processing_init();
            G_bolos_ux_context.processing = 1;
    }
  }
  else {
        // add a space before next word
    G_bolos_ux_context.words_buffer[G_bolos_ux_context.words_buffer_length++] = ' ';

        // enter the next word
    /*indexes have been preincremented, it's therefore the next word we're reentering*/
    screen_onboarding_4_restore_word_init(RESTORE_WORD_ACTION_REENTER_WORD);
    }
}


unsigned int screen_onboarding_4_restore_word_select_button(unsigned int button_mask, unsigned int button_mask_counter) {
    UNUSED(button_mask_counter);
  switch(button_mask) {
    case BUTTON_EVT_FAST | BUTTON_LEFT:
    case BUTTON_EVT_RELEASED | BUTTON_LEFT:

        // not a rotating list
      if (G_bolos_ux_context.hslider3_current == 0) {
            return 0;
        }
        bolos_ux_hslider3_previous();
        goto redraw;

    case BUTTON_EVT_FAST | BUTTON_RIGHT:
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        // not a rotating list
      if (G_bolos_ux_context.hslider3_current == G_bolos_ux_context.hslider3_total-1) {
            return 0;
        }
        bolos_ux_hslider3_next();

    redraw:
        screen_onboarding_4_restore_word_display_word_selection();
        break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
      if (G_bolos_ux_context.hslider3_current < G_bolos_ux_context.onboarding_words_checked) {
            // confirm word, and prepare entering a new one or validate the seed
            screen_onboarding_4_restore_word_validate();
        }
      else if (G_bolos_ux_context.hslider3_current == G_bolos_ux_context.onboarding_words_checked) {
        // clear current word
        screen_onboarding_4_restore_word_init(RESTORE_WORD_ACTION_REENTER_WORD);
    }
      else {
        // current word IS NOT already stored. need to wipe one less word
        // example selecting word #2: 0         1         2         3
        //                            prop1     prop2     edit #2   edit #1
        //                                                delete=0  delete=1
        unsigned int word_to_delete = G_bolos_ux_context.hslider3_current - G_bolos_ux_context.onboarding_words_checked;
        // remove x words
        while (word_to_delete--) {
          if (G_bolos_ux_context.onboarding_step && G_bolos_ux_context.words_buffer_length) {
            // remove the last space and up to the previous space (but keep the previous space)
            do {
              G_bolos_ux_context.words_buffer[G_bolos_ux_context.words_buffer_length-1] = 0;
              G_bolos_ux_context.words_buffer_length--;
}
            // until a previous word exists!
            while(G_bolos_ux_context.words_buffer_length && G_bolos_ux_context.words_buffer[G_bolos_ux_context.words_buffer_length-1] != ' ');

            // decrement onboarding_step (current word #)
            G_bolos_ux_context.onboarding_step--;
    }
    }
        // log_debug(G_bolos_ux_context.words_buffer);

        // clear previous word
        screen_onboarding_4_restore_word_init(RESTORE_WORD_ACTION_REENTER_WORD);
      }
      break;
  }
    return 0;
}

void screen_onboarding_4_restore_word_init(unsigned int action) {
  switch(action) {
    case RESTORE_WORD_ACTION_FIRST_WORD:
        // start by restore first word (+1 when displayed)
        G_bolos_ux_context.onboarding_step = 0;

        // flush the words first
      memset(G_bolos_ux_context.words_buffer, 0, sizeof(G_bolos_ux_context.words_buffer));
        G_bolos_ux_context.words_buffer_length = 0;
      break;

    case RESTORE_WORD_ACTION_REENTER_WORD:
      // don't change anything (the currently edited word has not been copied to the word buffer)
      break;
    }

  memset(G_ux.string_buffer, 0, sizeof(G_ux.string_buffer));
    // offset 0: the display buffer for various placement
    // offset 16: the entered stem for the current word restoration
  // offset 32: array of next letters possible after the current word's stem in the dictionary (word completion possibilities)

  // elements to be displayed
  SPRINTF(G_ux.string_buffer, "word #%d", G_bolos_ux_context.onboarding_step + 1);

  ux_flow_init(0, restore_4_intro, NULL);
}

#endif
