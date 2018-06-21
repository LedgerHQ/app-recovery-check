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

//#ifdef OS_IO_SEPROXYHAL

const bagl_element_t screen_onboarding_3_restore_0[] = {
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

    {{BAGL_LABELINE, 0x00, 0, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "Enter your",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x00, 0, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "recovery phrase",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};

const bagl_element_t screen_onboarding_3_restore_1[] = {
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

    {{BAGL_LABELINE, 0x00, 0, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "Select the number of",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x00, 0, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "words to restore",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};

const bagl_element_t screen_onboarding_3_restore_2[] = {
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

    {{BAGL_LABELINE, 0x01, 23, 14, 14, 9, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     G_bolos_ux_context.string_buffer,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LINE, 0x00, 44, 10, 3, 1, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x02, 49, 16, 30, 32, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_LIGHT_16px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     G_bolos_ux_context.string_buffer,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LINE, 0x00, 81, 10, 3, 1, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x03, 91, 14, 14, 9, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     G_bolos_ux_context.string_buffer,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x00, 0, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "words",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    // left/rights icons
    {{BAGL_ICON, 0x04, 3, 12, 4, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_LEFT},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_ICON, 0x05, 121, 12, 4, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_RIGHT},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};

const char *const screen_onboarding_3_restore_choices[] = {
    "24", "18", "12",
};

const unsigned int const screen_onboarding_3_restore_choices_value[] = {
    24, 18, 12,
};

const bagl_element_t *
screen_onboarding_3_restore_2_before_element_display_callback(
    const bagl_element_t *element) {
    switch (element->component.userid) {
    case 0x01:
        G_bolos_ux_context.string_buffer[0] = 0;
        if (G_bolos_ux_context.hslider3_before != BOLOS_UX_HSLIDER3_NONE) {
            strcpy(G_bolos_ux_context.string_buffer,
                   (const char *)PIC(screen_onboarding_3_restore_choices
                                         [G_bolos_ux_context.hslider3_before]));
        }
        break;

    case 0x02:
        strcpy(G_bolos_ux_context.string_buffer,
               (const char *)PIC(
                   screen_onboarding_3_restore_choices[G_bolos_ux_context
                                                           .hslider3_current]));
        break;

    case 0x03:
        G_bolos_ux_context.string_buffer[0] = 0;
        if (G_bolos_ux_context.hslider3_after != BOLOS_UX_HSLIDER3_NONE) {
            strcpy(G_bolos_ux_context.string_buffer,
                   (const char *)PIC(screen_onboarding_3_restore_choices
                                         [G_bolos_ux_context.hslider3_after]));
        }
        break;

    // left button
    case 0x04:
        if (G_bolos_ux_context.hslider3_before == BOLOS_UX_HSLIDER3_NONE) {
            return NULL; // don't display
        }
        break;

    // right button
    case 0x05:
        if (G_bolos_ux_context.hslider3_after == BOLOS_UX_HSLIDER3_NONE) {
            return NULL; // don't display
        }
        break;
    }

    // display other elements only if screen setup, else, only redraw words
    // value
    return element;
}

unsigned int
screen_onboarding_3_restore_2_button(unsigned int button_mask,
                                     unsigned int button_mask_counter) {
    UNUSED(button_mask_counter);
    switch (button_mask) {
    case BUTTON_EVT_FAST | BUTTON_LEFT:
    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        bolos_ux_hslider3_previous();
        goto redraw;

    case BUTTON_EVT_FAST | BUTTON_RIGHT:
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        bolos_ux_hslider3_next();

    redraw:
        
        UX_DISPLAY(screen_onboarding_3_restore_2, screen_onboarding_3_restore_2_before_element_display_callback);

        break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:

        // set the restore number of words
        G_bolos_ux_context.onboarding_kind =
            screen_onboarding_3_restore_choices_value[G_bolos_ux_context
                                                          .hslider3_current];

        // start recovery of the first word
        //PRINTF("CHOICE: %d\n", G_bolos_ux_context.onboarding_kind);
        screen_onboarding_4_restore_word_init(1);
        break;
    }
    return 0;
}

unsigned int
screen_onboarding_3_restore_1_button(unsigned int button_mask,
                                     unsigned int button_mask_counter) {
    UNUSED(button_mask_counter);
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:

        // initialize the slider with 3 entry
        bolos_ux_hslider3_init(3);

        UX_DISPLAY(screen_onboarding_3_restore_2, screen_onboarding_3_restore_2_before_element_display_callback);

        break;
    }
    return 0;
}

unsigned int
screen_onboarding_3_restore_0_button(unsigned int button_mask,
                                     unsigned int button_mask_counter) {
    UNUSED(button_mask_counter);
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:

        
        UX_DISPLAY(screen_onboarding_3_restore_1, NULL);


        break;
    }
    return 0;
}

extern unsigned char app_stack_canary;



void screen_onboarding_3_restore_init(void) {

    app_stack_canary = 222;
    //PRINTF("START! %d\n", app_stack_canary);
    

    // elements to be displayed
    UX_DISPLAY(screen_onboarding_3_restore_0, NULL);

}

//#endif // OS_IO_SEPROXYHAL
