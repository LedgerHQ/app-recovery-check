/*******************************************************************************
 *   (c) 2016-2022 Ledger SAS
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

#include <lcx_hmac.h>
#include <lcx_rng.h>

#include "../constants.h"
#include "../ui.h"

#if defined(TARGET_NANOX) || defined(TARGET_NANOS2)

const bagl_element_t* screen_onboarding_4_restore_word_before_element_display_callback(
    const bagl_element_t* element);

// show intro
const bagl_element_t screen_onboarding_4_restore_word_intro_elements[] = {
    // erase
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 64, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0}, NULL},

    {{BAGL_LABELINE,
      0x31,
      0,
      12,
      128,
      32,
      0,
      0,
      0,
      0xFFFFFF,
      0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER,
      0},
     "Enter first letters"},
    {{BAGL_LABELINE,
      0x32,
      0,
      12,
      128,
      32,
      0,
      0,
      0,
      0xFFFFFF,
      0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER,
      0},
     "Next, enter letters"},
    {{BAGL_LABELINE,
      0x33,
      0,
      12,
      128,
      32,
      0,
      0,
      0,
      0xFFFFFF,
      0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER,
      0},
     "Finally, enter letters"},
    {{BAGL_LABELINE,
      0x30,
      0,
      26,
      128,
      32,
      0,
      0,
      0,
      0xFFFFFF,
      0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER,
      0},
     G_ux.string_buffer},
};

// word selection + word clear
const bagl_element_t screen_onboarding_4_restore_word_select_elements[] = {
    // erase
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 64, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0}, NULL},

    {{BAGL_LABELINE,
      0x21,
      0,
      29,
      128,
      32,
      0,
      0,
      0,
      0xFFFFFF,
      0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER,
      0},
     G_ux.string_buffer},
    {{BAGL_LABELINE,
      0x20,
      0,
      43,
      128,
      32,
      0,
      0,
      0,
      0xFFFFFF,
      0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,
      0},
     G_ux.string_buffer},

    {{BAGL_ICON, 0x24, (128 - 14) / 2, 17, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, 0, 0},
     (const char*) &C_icon_clear},
    {{BAGL_LABELINE,
      0x24,
      0,
      43,
      128,
      32,
      0,
      0,
      0,
      0xFFFFFF,
      0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,
      0},
     "Clear word"},

    // left/rights icons
    {{BAGL_ICON, 0x22, 2, 28, 4, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, 0}, (const char*) &C_icon_left},
    {{BAGL_ICON, 0x23, 122, 28, 4, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, 0},
     (const char*) &C_icon_right},
};

UX_STEP_NOCB(ux_load_step, pn, {&C_icon_loader, "Processing"});
UX_FLOW(ux_load_flow, &ux_load_step);

UX_STEP_VALID(ux_wrong_seed_step,
              pnn,
              os_sched_exit(-1),
              {
                  &C_icon_crossmark,
                  "Recovery",
                  "phrase invalid",
              });
UX_FLOW(ux_wrong_seed_flow, &ux_wrong_seed_step);

UX_STEP_NOCB(ux_failed_check_step_1, pbb, {&C_icon_warning, "Phrase", "doesn't match"});
UX_STEP_NOCB(ux_failed_check_step_2,
             nn,
             {
                 "Check length",
                 "order and spelling",
             });
UX_STEP_VALID(ux_failed_check_step_3, pb, ui_idle_init(), {&C_icon_back_x, "Return to menu"});
UX_FLOW(ux_failed_check_flow,
        &ux_failed_check_step_1,
        &ux_failed_check_step_2,
        &ux_failed_check_step_3);

UX_STEP_VALID(ux_success_step,
              pbb,
              os_sched_exit(-1),
              {&C_icon_validate_14, "Phrase", "is correct"});
UX_FLOW(ux_succesfull_check_flow, &ux_success_step);

unsigned int screen_onboarding_4_restore_word_select_button(unsigned int button_mask,
                                                            unsigned int button_mask_counter);

const bagl_element_t* screen_onboarding_4_restore_word_keyboard_callback(unsigned int event,
                                                                         unsigned int value);

void screen_onboarding_4_restore_word_display_auto_complete(void) {
    unsigned int auto_complete_count = bolos_ux_bip39_get_word_next_letters_starting_with(
        (unsigned char*) G_ux.string_buffer + 16,
        strlen(G_ux.string_buffer + 16),
        (unsigned char*) G_ux.string_buffer + 32);
    // prepare title of the common keyboard component, after the list of possible letters
    snprintf(G_ux.string_buffer + 32 + auto_complete_count + 1,
             sizeof(G_ux.string_buffer) - 32 - auto_complete_count - 1,
             "Enter word #%d",
             G_bolos_ux_context.onboarding_step + 1);
    // prepare to display the backspace
    G_ux.string_buffer[32 + auto_complete_count] = '\b';
    G_bolos_ux_context.common_label = G_ux.string_buffer + 32 + auto_complete_count + 1;
    // display added letter and refresh slider
    screen_common_keyboard_init(
        0,
#ifdef HAVE_BOLOS_NOT_SHUFFLED_RESTORE
        0, /*always restart from the first element in the list*/
#else  // HAVE_BOLOS_NOT_SHUFFLED_RESTORE
        (strlen(G_ux.string_buffer + 16)
             ? 0
             : cx_rng_u8() % auto_complete_count), /* start from a random element in the list for
                                                      the word start letter, else keep the order */
#endif  // HAVE_BOLOS_NOT_SHUFFLED_RESTORE
        // recompute alphabet and set the number of elements in the keyboard
        auto_complete_count +
            (strlen(G_ux.string_buffer + 16)
                 ? 1
                 : 0) /* backspace if a stem is already entered, else no backspace */,
        screen_onboarding_4_restore_word_keyboard_callback);
    // append the special backspace to allow for easier dispatch in the keyboard callback
    ((unsigned char*) (G_ux.string_buffer + 32))[auto_complete_count] = '\b';
}

void screen_onboarding_4_restore_word_display_word_selection(void) {
    ux_stack_init(0);
    G_ux.stack[0].button_push_callback = screen_onboarding_4_restore_word_select_button;
    G_ux.stack[0].element_arrays[0].element_array =
        screen_onboarding_4_restore_word_select_elements;
    G_ux.stack[0].element_arrays[0].element_array_count =
        ARRAYLEN(screen_onboarding_4_restore_word_select_elements);
    G_ux.stack[0].element_arrays_count = 1;
    G_ux.stack[0].screen_before_element_display_callback =
        screen_onboarding_4_restore_word_before_element_display_callback;
    ux_stack_display(0);
}

const bagl_element_t* screen_onboarding_4_restore_word_keyboard_callback(const unsigned int event,
                                                                         const unsigned int value) {
    switch (event) {
        case KEYBOARD_ITEM_VALIDATED:
            // depending on the chosen class, interpret the click
            if (G_ux.string_buffer[32 + value] == '\b') {
                if (strlen(G_ux.string_buffer + 16)) {
                    G_ux.string_buffer[16 + strlen(G_ux.string_buffer + 16) - 1] = 0;
                    screen_onboarding_4_restore_word_display_auto_complete();
                }
            } else {
                unsigned int nb_words_matching_stem;
                // validate next letter of the word
                G_ux.string_buffer[16 + strlen(G_ux.string_buffer + 16)] =
                    G_ux.string_buffer[32 + G_bolos_ux_context.hslider3_current];

                // continue displaying until less than X words matches the stem
                nb_words_matching_stem = bolos_ux_bip39_get_word_count_starting_with(
                    (unsigned char*) G_ux.string_buffer + 16,
                    strlen(G_ux.string_buffer + 16));
                if (nb_words_matching_stem > ONBOARDING_WORD_COMPLETION_MAX_ITEMS) {
                    // too much words for slider word completion, await another letter
                    screen_onboarding_4_restore_word_display_auto_complete();
                } else {
                    // always init stem count
                    // index of the first word matching the stem
                    G_bolos_ux_context.onboarding_index = bolos_ux_bip39_get_word_idx_starting_with(
                        (unsigned char*) G_ux.string_buffer + 16,
                        strlen(G_ux.string_buffer + 16));

                    // multiple possibilities
                    // update the slider's possible words
                    // account for the extra "last" (clear) item
                    bolos_ux_hslider3_init(nb_words_matching_stem + 1);
                    screen_onboarding_4_restore_word_display_word_selection();
                }
                return NULL;
            }
            break;

        case KEYBOARD_RENDER_ITEM:
            if (G_ux.string_buffer[32 + value] == '\b') {
                /*
                x:57 w:14
                iconw: 11
                */
                G_ux.tmp_element.component.width = C_icon_backspace.width;
                G_ux.tmp_element.component.x +=
                    1 + G_ux.tmp_element.component.width / 2 - C_icon_backspace.width / 2;
                G_ux.tmp_element.component.y -= 7;
                // G_ux.tmp_element.component.y = 5;
                G_ux.tmp_element.component.height = C_icon_backspace.height;
                G_ux.tmp_element.component.type = BAGL_ICON;
                G_ux.tmp_element.component.icon_id = 0;
                if (G_ux.tmp_element.component.userid == 0x02) {
                    G_ux.tmp_element.text = (const char*) &C_icon_backspace_invert;
                } else {
                    G_ux.tmp_element.text = (const char*) &C_icon_backspace;
                }
            } else {
                // G_ux.string_buffer[0] = G_ux.string_buffer[32+value]-'a'+'A'; // render as
                // uppercase, always
                G_ux.string_buffer[0] =
                    G_ux.string_buffer[32 + value];  // render as lowercase, always
                G_ux.string_buffer[1] = 0;
                G_ux.tmp_element.text = G_ux.string_buffer;
            }
            break;
        case KEYBOARD_RENDER_WORD: {
            unsigned int l = strlen(G_ux.string_buffer + 16);

            if (value < 8) {
                G_ux.tmp_element.component.x += 5;
                // ensure font is left aligned
                G_ux.tmp_element.text = G_ux.string_buffer;
                G_ux.string_buffer[1] = 0;
                if (l < 8) {
                    if (l > value) {
                        G_ux.string_buffer[0] = G_ux.string_buffer[16 + value];
                    } else {
                        G_ux.string_buffer[0] = '_';
                    }
                } else {
                    // first char is '...' to notify continuing
                    if (value == 0) {
                        G_ux.string_buffer[0] = '.';
                        G_ux.string_buffer[1] = '.';
                        G_ux.string_buffer[2] = '.';
                        G_ux.string_buffer[3] = 0;
                    } else {
                        G_ux.string_buffer[0] = (G_ux.string_buffer + 16 + l + 1 - 8)[value];
                    }
                }
                break;
            }
            return NULL;
        }
    }
    // update element display
    return &G_ux.tmp_element;
}

const bagl_element_t* screen_onboarding_4_restore_word_before_element_display_callback(
    const bagl_element_t* element) {
    switch (element->component.userid) {
        case 0x30:
            // word index
            SPRINTF(G_ux.string_buffer, "of word #%d", G_bolos_ux_context.onboarding_step + 1);
            break;

        case 0x31:
            if (G_bolos_ux_context.onboarding_step != 0) {
                return NULL;  // don't display
            }
            break;
        case 0x32:
            if (G_bolos_ux_context.onboarding_step == 0 ||
                G_bolos_ux_context.onboarding_step == G_bolos_ux_context.onboarding_kind - 1) {
                return NULL;  // don't display
            }
            break;
        case 0x33:
            if (G_bolos_ux_context.onboarding_step != G_bolos_ux_context.onboarding_kind - 1) {
                return NULL;  // don't display
            }
            break;

        case 0x20:
            // display matching word from the slider's current index
            bolos_ux_bip39_idx_strcpy(
                G_bolos_ux_context.onboarding_index + G_bolos_ux_context.hslider3_current,
                (unsigned char*) G_ux.string_buffer);
            goto not_on_last_item;

        case 0x21:
            SPRINTF(G_ux.string_buffer, "Select word #%d", G_bolos_ux_context.onboarding_step + 1);
            goto not_on_last_item;

        case 0x22:
            // no left button on first item
            if (G_bolos_ux_context.hslider3_current == 0) {
                return NULL;
            }
            break;
        not_on_last_item:
        case 0x23:
            // no right button on last item
            if (G_bolos_ux_context.hslider3_total == 0 ||
                G_bolos_ux_context.hslider3_current == G_bolos_ux_context.hslider3_total - 1) {
                return NULL;
            }
            break;

        case 0x24:
            if (G_bolos_ux_context.hslider3_total == 0 ||
                G_bolos_ux_context.hslider3_current != G_bolos_ux_context.hslider3_total - 1) {
                return NULL;
            }
            break;
        case 0x25:
            // word index
            SPRINTF(G_ux.string_buffer, "Enter word #%d", G_bolos_ux_context.onboarding_step + 1);
            break;
    }

    // display other elements only if screen setup, else, only redraw words value
    return element;
}

uint8_t compare_recovery_phrase(void) {
    // io_seproxyhal_general_status();

    // convert mnemonic to hex-seed
    uint8_t buffer[64];

    bolos_ux_mnemonic_to_seed((unsigned char*) G_bolos_ux_context.words_buffer,
                              G_bolos_ux_context.words_buffer_length,
                              buffer);
    PRINTF("Input seed:\n %.*H\n", 64, buffer);

    // get rootkey from hex-seed
    cx_hmac_sha512_t ctx;
    const char key[] = "Bitcoin seed";

    cx_hmac_sha512_init(&ctx, (const uint8_t*) key, strlen(key));
    cx_hmac((cx_hmac_t*) &ctx, CX_LAST, buffer, 64, buffer, 64);
    PRINTF("Root key from input:\n%.*H\n", 64, buffer);

    // get rootkey from device's seed
    uint8_t buffer_device[64];

    os_perso_derive_node_bip32(CX_CURVE_256K1, NULL, 0, buffer_device, buffer_device + 32);
    PRINTF("Root key from device: \n%.*H\n", 64, buffer_device);

    // compare both rootkey
    return os_secure_memcmp(buffer, buffer_device, 64) ? 0 : 1;
}

void screen_onboarding_4_restore_word_validate(void) {
    bolos_ux_bip39_idx_strcpy(
        G_bolos_ux_context.onboarding_index + G_bolos_ux_context.hslider3_current,
        (unsigned char*) (G_bolos_ux_context.words_buffer +
                          G_bolos_ux_context.words_buffer_length));
    G_bolos_ux_context.words_buffer_length = strlen(G_bolos_ux_context.words_buffer);

    // a word has been added
    G_bolos_ux_context.onboarding_step++;

    if (G_bolos_ux_context.onboarding_step == G_bolos_ux_context.onboarding_kind) {
        unsigned char valid;
#ifdef HAVE_ELECTRUM
        // if we've entered all the words, then check the phrase
        if (G_bolos_ux_context.onboarding_algorithm == BOLOS_UX_ONBOARDING_ALGORITHM_ELECTRUM) {
            valid =
                bolos_ux_electrum_mnemonic_check(ELECTRUM_SEED_PREFIX_STANDARD,
                                                 (unsigned char*) G_bolos_ux_context.words_buffer,
                                                 G_bolos_ux_context.words_buffer_length);
        } else {
            valid = bolos_ux_mnemonic_check((unsigned char*) G_bolos_ux_context.words_buffer,
                                            G_bolos_ux_context.words_buffer_length);
        }
#else
        valid = bolos_ux_mnemonic_check((unsigned char*) G_bolos_ux_context.words_buffer,
                                        G_bolos_ux_context.words_buffer_length);
#endif
        if (!valid) {
            // invalid recovery phrase
            ux_flow_init(0, ux_wrong_seed_flow, NULL);
        } else {
            // alright, the recovery phrase looks ok, compare it to onboarded seed

            // Display loading icon to user
            ux_flow_init(0, ux_load_flow, NULL);
            if (compare_recovery_phrase()) {
                ux_flow_init(0, ux_succesfull_check_flow, NULL);
            } else {
                ux_flow_init(0, ux_failed_check_flow, NULL);
            }
        }
    } else {
        // add a space before next word
        G_bolos_ux_context.words_buffer[G_bolos_ux_context.words_buffer_length++] = ' ';

        // enter the next word
        screen_onboarding_4_restore_word_init(0);
    }
}

unsigned int screen_onboarding_4_restore_word_select_button(
    const unsigned int button_mask,
    const unsigned int button_mask_counter) {
    UNUSED(button_mask_counter);
    switch (button_mask) {
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
            if (G_bolos_ux_context.hslider3_current == G_bolos_ux_context.hslider3_total - 1) {
                return 0;
            }
            bolos_ux_hslider3_next();

        redraw:
            screen_onboarding_4_restore_word_display_word_selection();
            break;

        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            if (G_bolos_ux_context.hslider3_current == G_bolos_ux_context.hslider3_total - 1) {
                // clear current word
                screen_onboarding_4_restore_word_init(0);
            } else {
                // confirm word, and prepare entering a new one or validate the seed
                screen_onboarding_4_restore_word_validate();
            }
            break;
    }
    return 0;
}

unsigned int screen_onboarding_4_restore_word_intro_button(const unsigned int button_mask,
                                                           const unsigned int button_mask_counter) {
    UNUSED(button_mask_counter);
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: {
            screen_onboarding_4_restore_word_display_auto_complete();
            break;
        }
    }
    return 0;
}

void screen_onboarding_4_restore_word_init(unsigned int firstWord) {
    ux_stack_init(0);

    if (firstWord) {
        // start by restore first word (+1 when displayed)
        G_bolos_ux_context.onboarding_step = 0;

        // flush the words first
        memset(G_bolos_ux_context.words_buffer, 0, sizeof(G_bolos_ux_context.words_buffer));
        G_bolos_ux_context.words_buffer_length = 0;
    }

    memset(G_ux.string_buffer, 0, sizeof(G_ux.string_buffer));
    // offset 0: the display buffer for various placement
    // offset 16: the entered stem for the current word restoration
    // offset 32: array of next letters possible after the current word's stem in the dictionary
    // (word completion possibilities)

#ifdef RESTORE_INTRO_WORD
    G_ux.stack[0].button_push_callback = screen_onboarding_4_restore_word_intro_button;
    G_ux.stack[0].screen_before_element_display_callback =
        screen_onboarding_4_restore_word_before_element_display_callback;

    // elements to be displayed
    G_ux.stack[0].element_arrays[0].element_array = screen_onboarding_4_restore_word_intro_elements;
    G_ux.stack[0].element_arrays[0].element_array_count =
        ARRAYLEN(screen_onboarding_4_restore_word_intro_elements);
    G_ux.stack[0].element_arrays_count = 1;
    ux_stack_display(0);
#else  // RESTORE_INTRO_WORD
    screen_onboarding_4_restore_word_display_auto_complete();
#endif  // RESTORE_INTRO_WORD
}

#endif
