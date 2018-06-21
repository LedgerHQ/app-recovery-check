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

#include "os.h"
#include "cx.h"

#include "os_io_seproxyhal.h"
#include "string.h"

#include "bolos_ux_common.h"

#include "glyphs.h"

//#ifdef OS_IO_SEPROXYHAL

const bagl_element_t *
screen_onboarding_4_restore_word_before_element_display_callback(
    const bagl_element_t *element);

// show intro
const bagl_element_t screen_onboarding_4_restore_word_intro[] = {
    // erase
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_LABELINE, 0x31, 0, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "Enter first letters",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x32, 0, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "Next, enter letters",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x33, 0, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "Finally, enter letters",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x30, 0, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     G_bolos_ux_context.string_buffer,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};

// word selection + word clear
const bagl_element_t screen_onboarding_4_restore_word_select[] = {
    // erase
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_LABELINE, 0x21, 0, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     G_bolos_ux_context.string_buffer,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x20, 0, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     G_bolos_ux_context.string_buffer,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x24, 23, 9, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CROSS_BADGE},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x24, 43, 19, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px, 0},
     "Clear word",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    // left/rights icons
    {{BAGL_ICON, 0x22, 3, 12, 4, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_LEFT},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_ICON, 0x23, 121, 12, 4, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_RIGHT},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};

// recovery phrase error
const bagl_element_t screen_onboarding_4_restore_word_invalid[] = {
    // erase
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x00, 6, 9, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CROSS_BADGE},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x00, 27, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px, 0},
     "Recovery phrase",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x00, 27, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px, 0},
     "is not valid",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};

// seed comparison result
const bagl_element_t screen_onboarding_4_restore_word_final_compare[] = {
    // erase
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x01, 6, 9, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CROSS_BADGE},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
     {{BAGL_ICON, 0x02, 6, 9, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CHECK_BADGE},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x00, 10, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "Recovery phrase",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x00, 10, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     G_bolos_ux_context.string_buffer,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};

// processing warning

const bagl_element_t screen_onboarding_4_processing[] = {
    // erase
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x01, 11, 9, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_LOADING_BADGE},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_LABELINE, 0x00, 10, 20, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER | BAGL_FONT_ALIGNMENT_MIDDLE, 0},
     "Processing...",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL}
};

unsigned int screen_onboarding_4_restore_word_select_button(
    unsigned int button_mask, unsigned int button_mask_counter);

#define ITEMS (G_bolos_ux_context.string_buffer + 32)

const bagl_element_t *
screen_onboarding_4_restore_word_keyboard_callback(unsigned int event,
                                                   unsigned int value);

void screen_onboarding_4_restore_word_display_auto_complete(void)
{
    unsigned int auto_complete_count =
        bolos_ux_bip39_get_word_next_letters_starting_with(
            (unsigned char *)G_bolos_ux_context.string_buffer + 16,
            strlen(G_bolos_ux_context.string_buffer + 16),
            (unsigned char *)G_bolos_ux_context.string_buffer + 32);
    // display added letter and refresh slider
    screen_common_keyboard_init(
        0, 0, /*always restart from the first element in the list*/
        // recompute alphabet and set the number of elements in the keyboard
        auto_complete_count + (strlen(G_bolos_ux_context.string_buffer + 16)
                                   ? 1
                                   : 0) /* backspace */,
        screen_onboarding_4_restore_word_keyboard_callback);
    // append the special backspace to allow for easier dispatch in the keyboard
    // callback
    ((unsigned char *)(G_bolos_ux_context.string_buffer +
                       32))[auto_complete_count] = '\b';
}

void screen_onboarding_4_restore_word_display_word_selection(void)
{
    /*screen_state_init(0);
    G_bolos_ux_context.screen_stack[0].button_push_callback =
        screen_onboarding_4_restore_word_select_button;
    G_bolos_ux_context.screen_stack[0].element_arrays[0].element_array =
        screen_onboarding_4_restore_word_select_elements;
    G_bolos_ux_context.screen_stack[0].element_arrays[0].element_array_count =
        ARRAYLEN(screen_onboarding_4_restore_word_select_elements);
    G_bolos_ux_context.screen_stack[0].element_arrays_count = 1;
    G_bolos_ux_context.screen_stack[0].screen_before_element_display_callback =
        screen_onboarding_4_restore_word_before_element_display_callback;
    screen_display_init(0);*/

    UX_DISPLAY(screen_onboarding_4_restore_word_select, screen_onboarding_4_restore_word_before_element_display_callback);
}

const bagl_element_t * 
screen_onboarding_4_processing_before_element_display_callback(const bagl_element_t *element) 
{
    // copy element to be displayed
    os_memmove(&G_bolos_ux_context.tmp_element, PIC(element),
               sizeof(G_bolos_ux_context.tmp_element));

    if(element->component.userid == 0x01)
    {
        G_bolos_ux_context.tmp_element.component.width =
            C_badge_loading_v2.width;
        
        G_bolos_ux_context.tmp_element.component.y = 9;
        G_bolos_ux_context.tmp_element.component.height =
            C_badge_loading_v2.height;
        G_bolos_ux_context.tmp_element.component.type = BAGL_ICON;
        G_bolos_ux_context.tmp_element.component.icon_id = 0;
        G_bolos_ux_context.tmp_element.text =
            (const char *)&C_badge_loading_v2;
    }
    // update element display
    return &G_bolos_ux_context.tmp_element;
}

const bagl_element_t *
screen_onboarding_4_restore_word_keyboard_callback(unsigned int event,
                                                   unsigned int value)
{
    //PRINTF("Z %d %d\n", event, value);
    switch (event)
    {
    case KEYBOARD_ITEM_VALIDATED:
        // depending on the chosen class, interpret the click
        if (G_bolos_ux_context.string_buffer[32 + value] == '\b')
        {
            if (strlen(G_bolos_ux_context.string_buffer + 16))
            {
                G_bolos_ux_context.string_buffer
                    [16 + strlen(G_bolos_ux_context.string_buffer + 16) - 1] =
                    0;
                screen_onboarding_4_restore_word_display_auto_complete();
            }
        }
        else
        {
            unsigned int nb_words_matching_stem;
            // validate next letter of the word
            G_bolos_ux_context
                .string_buffer[16 +
                               strlen(G_bolos_ux_context.string_buffer + 16)] =
                G_bolos_ux_context
                    .string_buffer[32 + G_bolos_ux_context.hslider3_current];

            // continue displaying until less than X words matches the stem
            nb_words_matching_stem =
                bolos_ux_bip39_get_word_count_starting_with(
                    (unsigned char *)G_bolos_ux_context.string_buffer + 16,
                    strlen(G_bolos_ux_context.string_buffer + 16));
            if (nb_words_matching_stem > ONBOARDING_WORD_COMPLETION_MAX_ITEMS)
            {
                // too much words for slider word completion, await another
                // letter
                screen_onboarding_4_restore_word_display_auto_complete();
            }
            else
            {
                // always init stem count
                // index of the first word matching the stem
                G_bolos_ux_context.onboarding_index =
                    bolos_ux_bip39_get_word_idx_starting_with(
                        (unsigned char *)G_bolos_ux_context.string_buffer + 16,
                        strlen(G_bolos_ux_context.string_buffer + 16));

                // multiple possibilities
                // update te slider's possible words
                // account for the extra "last" (clear) item
                bolos_ux_hslider3_init(nb_words_matching_stem + 1);
                screen_onboarding_4_restore_word_display_word_selection();
            }
            return NULL;
        }
        break;

    case KEYBOARD_RENDER_ITEM:
        if (G_bolos_ux_context.string_buffer[32 + value] == '\b')
        {
            /*
            x:57 w:14
            iconw: 11
            */
            value = 3;
            G_bolos_ux_context.tmp_element.component.width =
                C_icon_backspace.width;
            G_bolos_ux_context.tmp_element.component.x +=
                1 + G_bolos_ux_context.tmp_element.component.width / 2 -
                C_icon_backspace.width / 2;
            // G_bolos_ux_context.tmp_element.component.y +=
            // screen_settings_passphrase_classes_icons[value]->height/2 -
            // G_bolos_ux_context.tmp_element.component.height/2;
            G_bolos_ux_context.tmp_element.component.y = 5;
            G_bolos_ux_context.tmp_element.component.height =
                C_icon_backspace.height;
            G_bolos_ux_context.tmp_element.component.type = BAGL_ICON;
            G_bolos_ux_context.tmp_element.component.icon_id = 0;
            G_bolos_ux_context.tmp_element.text =
                (const char *)&C_icon_backspace;
        }
        else
        {
            G_bolos_ux_context.string_buffer[0] =
                G_bolos_ux_context.string_buffer[32 + value] - 'a' +
                'A'; // render as uppercase, always
            G_bolos_ux_context.string_buffer[1] = 0;
            G_bolos_ux_context.tmp_element.text =
                G_bolos_ux_context.string_buffer;
        }
        break;
    case KEYBOARD_RENDER_WORD:
    {
        unsigned int l = strlen(G_bolos_ux_context.string_buffer + 16);

        if (value < 8)
        {
            G_bolos_ux_context.tmp_element.component.x += 5;
            // ensure font is left aligned
            G_bolos_ux_context.tmp_element.text =
                G_bolos_ux_context.string_buffer;
            G_bolos_ux_context.string_buffer[1] = 0;
            if (l < 8)
            {
                if (l > value)
                {
                    G_bolos_ux_context.string_buffer[0] =
                        G_bolos_ux_context.string_buffer[16 + value];
                }
                else
                {
                    G_bolos_ux_context.string_buffer[0] = '_';
                }
            }
            else
            {
                // first char is '...' to notify continuing
                if (value == 0)
                {
                    G_bolos_ux_context.string_buffer[0] = '.';
                    G_bolos_ux_context.string_buffer[1] = '.';
                    G_bolos_ux_context.string_buffer[2] = '.';
                    G_bolos_ux_context.string_buffer[3] = 0;
                }
                else
                {
                    G_bolos_ux_context.string_buffer[0] =
                        (G_bolos_ux_context.string_buffer + 16 + l + 1 -
                         8)[value];
                }
            }
            break;
        }
        return NULL;
    }
    }
    // update element display
    return &G_bolos_ux_context.tmp_element;
}

unsigned int screen_onboarding_4_restore_word_final_compare_button(
    unsigned int button_mask, unsigned int button_mask_counter)
{
    UNUSED(button_mask_counter);
    switch (button_mask)
    {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
        os_sched_exit(0);
        break;
    }
    return 0;
}

const bagl_element_t *
screen_onboarding_4_restore_word_final_compare_before_element_display_callback(
    const bagl_element_t *element)
{
     switch (element->component.userid)
    {
    case 0x01:
        if(!G_bolos_ux_context.input_seed_is_identical)
        {
            SPRINTF(G_bolos_ux_context.string_buffer, "doesn't match!");
        }
        else
        {
            return NULL; // don't display
        }
        break;
    case 0x02:
        if(G_bolos_ux_context.input_seed_is_identical)
        {
            SPRINTF(G_bolos_ux_context.string_buffer, "matches");
        }
        else
        {
            return NULL; // don't display
        }
        break;
    }

    return element;
}

const bagl_element_t *
screen_onboarding_4_restore_word_before_element_display_callback(
    const bagl_element_t *element)
{
    switch (element->component.userid)
    {
    case 0x30:
        // word index
        SPRINTF(G_bolos_ux_context.string_buffer, "of word #%d",
                G_bolos_ux_context.onboarding_step + 1);
        break;

    case 0x31:
        if (G_bolos_ux_context.onboarding_step != 0)
        {
            return NULL; // don't display
        }
        break;
    case 0x32:
        if (G_bolos_ux_context.onboarding_step == 0 ||
            G_bolos_ux_context.onboarding_step ==
                G_bolos_ux_context.onboarding_kind - 1)
        {
            return NULL; // don't display
        }
        break;
    case 0x33:
        if (G_bolos_ux_context.onboarding_step !=
            G_bolos_ux_context.onboarding_kind - 1)
        {
            return NULL; // don't display
        }
        break;

    case 0x20:
        // display matching word from the slider's current index
        bolos_ux_bip39_idx_strcpy(
            G_bolos_ux_context.onboarding_index +
                G_bolos_ux_context.hslider3_current,
            (unsigned char *)G_bolos_ux_context.string_buffer);
        goto not_on_last_item;

    case 0x21:
        SPRINTF(G_bolos_ux_context.string_buffer, "Select word #%d",
                G_bolos_ux_context.onboarding_step + 1);
        goto not_on_last_item;

    case 0x22:
        // no left button on first item
        if (G_bolos_ux_context.hslider3_current == 0)
        {
            return NULL;
        }
        break;
    not_on_last_item:
    case 0x23:
        // no right button on last item
        if (G_bolos_ux_context.hslider3_total == 0 ||
            G_bolos_ux_context.hslider3_current ==
                G_bolos_ux_context.hslider3_total - 1)
        {
            return NULL;
        }
        break;

    case 0x24:
        if (G_bolos_ux_context.hslider3_total == 0 ||
            G_bolos_ux_context.hslider3_current !=
                G_bolos_ux_context.hslider3_total - 1)
        {
            return NULL;
        }
        break;
    }

    // display other elements only if screen setup, else, only redraw words
    // value
    return element;
}


unsigned int screen_onboarding_4_processing_button(
    unsigned int button_mask, unsigned int button_mask_counter){
        // No interaction here
    }

unsigned int screen_onboarding_4_restore_word_invalid_button(
    unsigned int button_mask, unsigned int button_mask_counter)
{
    UNUSED(button_mask_counter);
    switch (button_mask)
    {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
        // enter a new recovery phrase
        screen_onboarding_3_restore_init();
        break;
    }
    return 0;
}


void foo(void)
{
    //PRINTF("HELLO\n");
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

    cx_hmac_sha512_init(&ctx, key, sizeof(key));
    cx_hmac(&ctx,CX_LAST, buffer, 64, buffer);
    //PRINTF("Root key from input:\n%.*H\n", 64, buffer);

    // get rootkey from device's seed
    uint8_t buffer_device[64];
    const unsigned int path = "";

    os_perso_derive_node_bip32(CX_CURVE_256K1, path, 0, buffer_device, buffer_device+32);
    //PRINTF("Root key from device: \n%.*H\n", 64, buffer_device);

    // compare both rootkey
    G_bolos_ux_context.input_seed_is_identical = os_secure_memcmp(buffer, buffer_device, 64) ? 0:1;


    UX_DISPLAY(screen_onboarding_4_restore_word_final_compare, screen_onboarding_4_restore_word_final_compare_before_element_display_callback);
}


void screen_onboarding_4_restore_word_validate(void)
{
    bolos_ux_bip39_idx_strcpy(
        G_bolos_ux_context.onboarding_index +
            G_bolos_ux_context.hslider3_current,
        (unsigned char *)(G_bolos_ux_context.words_buffer +
                          G_bolos_ux_context.words_buffer_length));
    G_bolos_ux_context.words_buffer_length =
        strlen(G_bolos_ux_context.words_buffer);

    // a word has been added
    G_bolos_ux_context.onboarding_step++;

    if (G_bolos_ux_context.onboarding_step ==
        G_bolos_ux_context.onboarding_kind )//|| 1)
    {
        unsigned char valid;

        valid = bolos_ux_mnemonic_check(
            (unsigned char *)G_bolos_ux_context.words_buffer,
            G_bolos_ux_context.words_buffer_length);

            //valid = 1;

        if (!valid)
        {
            UX_DISPLAY(screen_onboarding_4_restore_word_invalid, NULL);
        }
        else
        {
            // allright, the recovery phrase looks ok, compare it to onboarded seed

            //Display processing warning to user
            UX_DISPLAY(screen_onboarding_4_processing, screen_onboarding_4_processing_before_element_display_callback);
            G_bolos_ux_context.processing = 1;

            
        }
    }
    else
    {
        // add a space before next word
        G_bolos_ux_context
            .words_buffer[G_bolos_ux_context.words_buffer_length++] = ' ';

        // enter the next word
        screen_onboarding_4_restore_word_init(0);
    }
}


unsigned int screen_onboarding_4_restore_word_select_button(
    unsigned int button_mask, unsigned int button_mask_counter)
{
    UNUSED(button_mask_counter);
    switch (button_mask)
    {
    case BUTTON_EVT_FAST | BUTTON_LEFT:
    case BUTTON_EVT_RELEASED | BUTTON_LEFT:

        // not a rotating list
        if (G_bolos_ux_context.hslider3_current == 0)
        {
            return 0;
        }
        bolos_ux_hslider3_previous();
        goto redraw;

    case BUTTON_EVT_FAST | BUTTON_RIGHT:
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        // not a rotating list
        if (G_bolos_ux_context.hslider3_current ==
            G_bolos_ux_context.hslider3_total - 1)
        {
            return 0;
        }
        bolos_ux_hslider3_next();

    redraw:
        screen_onboarding_4_restore_word_display_word_selection();
        break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
        if (G_bolos_ux_context.hslider3_current ==
            G_bolos_ux_context.hslider3_total - 1)
        {
            // clear current word
            screen_onboarding_4_restore_word_init(0);
        }
        else
        {
            // confirm word, and prepare entering a new one or validate the seed
            screen_onboarding_4_restore_word_validate();
        }
        break;
    }
    return 0;
}

unsigned int screen_onboarding_4_restore_word_intro_button(
    unsigned int button_mask, unsigned int button_mask_counter)
{
    UNUSED(button_mask_counter);
    switch (button_mask)
    {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
    {
        screen_onboarding_4_restore_word_display_auto_complete();
        break;
    }
    }
    return 0;
}

void screen_onboarding_4_restore_word_init(unsigned int firstWord)
{
    if (firstWord)
    {
        // start by restore first word (+1 when displayed)
        G_bolos_ux_context.onboarding_step = 0;

        // flush the words first
        os_memset(G_bolos_ux_context.words_buffer, 0,
                  sizeof(G_bolos_ux_context.words_buffer));
        G_bolos_ux_context.words_buffer_length = 0;
    }

    os_memset(G_bolos_ux_context.string_buffer, 0,
              sizeof(G_bolos_ux_context.string_buffer));
    // offset 0: the display buffer for various placement
    // offset 16: the entered stem for the current word restoration
    // offset 32: array of next letters possible after the current word's stem
    // in the dictionary (word completion possibilities)

    UX_DISPLAY(screen_onboarding_4_restore_word_intro, screen_onboarding_4_restore_word_before_element_display_callback);
}

//#endif // OS_IO_SEPROXYHAL
