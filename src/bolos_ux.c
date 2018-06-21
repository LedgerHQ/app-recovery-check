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

#define ARRAYLEN(array) (sizeof(array) / sizeof(array[0]))

bolos_ux_context_t G_bolos_ux_context;

unsigned short io_timeout(unsigned short last_timeout) {
    UNUSED(last_timeout);
    // infinite timeout
    return 1;
}

// check to process keyboard callback before screen generic callback
const bagl_element_t *
screen_display_element_callback(const bagl_element_t *element) {
    const bagl_element_t *el;
    if (G_bolos_ux_context.screen_stack_count) {
        if (G_bolos_ux_context
                .screen_stack[G_bolos_ux_context.screen_stack_count - 1]
                .screen_before_element_display_callback) {
            el = G_bolos_ux_context
                     .screen_stack[G_bolos_ux_context.screen_stack_count - 1]
                     .screen_before_element_display_callback(element);
            if (!el) {
                return 0;
            }
            if ((unsigned int)el != 1) {
                element = el;
            }
        }
    }
    // consider good to be displayed by default
    return element;
}

// return true (stack slot +1) if an element
unsigned int
screen_stack_is_element_array_present(const bagl_element_t *element_array) {
    unsigned int i, j;
    for (i = 0;
         i < /*ARRAYLEN(G_bolos_ux_context.screen_stack)*/ G_bolos_ux_context
                 .screen_stack_count;
         i++) {
        for (j = 0; j < G_bolos_ux_context.screen_stack[i].element_arrays_count;
             j++) {
            if (G_bolos_ux_context.screen_stack[i]
                    .element_arrays[j]
                    .element_array == element_array) {
                return i + 1;
            }
        }
    }
    return 0;
}
unsigned int screen_stack_push(void) {
    // only push if an available slot exists
    if (G_bolos_ux_context.screen_stack_count <
        ARRAYLEN(G_bolos_ux_context.screen_stack)) {
        os_memset(&G_bolos_ux_context
                       .screen_stack[G_bolos_ux_context.screen_stack_count],
                  0, sizeof(G_bolos_ux_context.screen_stack[0]));
        G_bolos_ux_context.screen_stack_count++;
    }
    // return the stack top index
    return G_bolos_ux_context.screen_stack_count - 1;
}
unsigned int screen_stack_pop(void) {
    unsigned int exit_code = BOLOS_UX_OK;
    // only pop if more than two stack entry (0 and 1,top is an index not a
    // count)
    if (G_bolos_ux_context.screen_stack_count > 0) {
        G_bolos_ux_context.screen_stack_count--;
        exit_code = G_bolos_ux_context
                        .screen_stack[G_bolos_ux_context.screen_stack_count]
                        .exit_code_after_elements_displayed;
        // wipe popped slot
        os_memset(&G_bolos_ux_context
                       .screen_stack[G_bolos_ux_context.screen_stack_count],
                  0, sizeof(G_bolos_ux_context.screen_stack[0]));
    }

    // prepare output code when popping the last stack screen
    if (G_bolos_ux_context.screen_stack_count == 0) {
        G_bolos_ux_context.exit_code = exit_code;
    }

    // ask for a complete redraw (optimisation due to blink must be avoided as
    // we're returning from a modal, and within the bolos ux screen stack)
    G_bolos_ux_context.screen_redraw = 1;
    // return the stack top index
    return G_bolos_ux_context.screen_stack_count - 1;
}

void screen_stack_remove(unsigned int stack_slot) {
    if (stack_slot > ARRAYLEN(G_bolos_ux_context.screen_stack) - 1) {
        stack_slot = ARRAYLEN(G_bolos_ux_context.screen_stack) - 1;
    }

    // removing something not in stack
    if (stack_slot >= G_bolos_ux_context.screen_stack_count) {
        return;
    }

    // before: | screenz | removed screen | other screenz |
    // after:  | screenz | other screenz |

    if (stack_slot != ARRAYLEN(G_bolos_ux_context.screen_stack) - 1) {
        os_memmove(
            &G_bolos_ux_context.screen_stack[stack_slot],
            &G_bolos_ux_context.screen_stack[stack_slot + 1],
            (ARRAYLEN(G_bolos_ux_context.screen_stack) - (stack_slot + 1)) *
                sizeof(G_bolos_ux_context.screen_stack[0]));
    }

    // wipe last slot
    screen_stack_pop();
}

void screen_display_element(const bagl_element_t *element) {
    const bagl_element_t *el = screen_display_element_callback(element);
    if (!el) {
        return;
    }
    if ((unsigned int)el != 1) {
        element = el;
    }
    // display current element
    io_seproxyhal_display(element);
}

const unsigned char const C_app_empty_colors[] = {
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
};

const unsigned char const C_app_empty_bitmap[] = {
    // color index table
    0x01, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,

    // icon mask
    0x00, 0x00, 0xF0, 0x0F, 0xFC, 0x3F, 0xFC, 0x3F, 0xFE, 0x7F, 0xFE, 0x7F,
    0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F, 0xFE, 0x7F,
    0xFC, 0x3F, 0xFC, 0x3F, 0xF0, 0x0F, 0x00, 0x00,
};

// prepare the app icon as if it was a icon_detail_t encoded structure in the
// string_buffer
void screen_prepare_masked_icon(unsigned char *icon_bitmap,
                                unsigned int icon_bitmap_length) {
    unsigned int i, inversemode;
    bagl_icon_details_t *icon_details =
        (bagl_icon_details_t *)G_bolos_ux_context.string_buffer;
    unsigned char *bitmap = (unsigned char *)G_bolos_ux_context.string_buffer +
                            sizeof(bagl_icon_details_t);

    icon_details->width = 16;
    icon_details->height = 16;
    // prepare the icon_details content
    icon_details->bpp = C_app_empty_bitmap[0];
    // use color table from the const
    icon_details->colors = (unsigned int *)C_app_empty_colors;
    icon_details->bitmap = bitmap;

    // when first color of the bitmap is not 0, then, must inverse the icon's
    // bit to
    // match the C_app_empty_bitmap bit value
    inversemode = 0;
    if (icon_bitmap[1] != 0 || icon_bitmap[2] != 0 || icon_bitmap[3] != 0 ||
        icon_bitmap[4] != 0) {
        inversemode = 1;
    }

    for (i = 1 + 8; i < sizeof(C_app_empty_bitmap) && i < icon_bitmap_length;
         i++) {
        if (inversemode) {
            bitmap[i - 1 - 8] = C_app_empty_bitmap[i] & (~icon_bitmap[i]);
        } else {
            bitmap[i - 1 - 8] = C_app_empty_bitmap[i] & icon_bitmap[i];
        }
    }

    // the string buffer is now ready to be displayed as an icon details
    // structure
}

void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default((bagl_element_t *)element);
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
        // G_bolos_ux_context.hslider3_before = 1; // full rotate
        G_bolos_ux_context.hslider3_current = 0;
        G_bolos_ux_context.hslider3_after = 1;
        break;
    default:
        G_bolos_ux_context.hslider3_before = total_count - 1;
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
        switch (G_bolos_ux_context.hslider3_current) {
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
        G_bolos_ux_context.hslider3_before =
            G_bolos_ux_context.hslider3_current;
        G_bolos_ux_context.hslider3_current = G_bolos_ux_context.hslider3_after;
        G_bolos_ux_context.hslider3_after =
            (G_bolos_ux_context.hslider3_after + 1) %
            G_bolos_ux_context.hslider3_total;
        break;
    }
}

void bolos_ux_hslider3_previous(void) {
    switch (G_bolos_ux_context.hslider3_total) {
    case 0:
    case 1:
        break;
    case 2:
        switch (G_bolos_ux_context.hslider3_current) {
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
        G_bolos_ux_context.hslider3_current =
            G_bolos_ux_context.hslider3_before;
        G_bolos_ux_context.hslider3_before =
            (G_bolos_ux_context.hslider3_before +
             G_bolos_ux_context.hslider3_total - 1) %
            G_bolos_ux_context.hslider3_total;
        break;
    }
}



//#endif // OS_IO_SEPROXYHAL
