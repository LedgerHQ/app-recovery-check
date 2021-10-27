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

#ifdef TARGET_NANOS

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

const unsigned char C_app_empty_colors[] = {
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
};

const unsigned char C_app_empty_bitmap[] = {
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
    icon_details->colors = (const unsigned int *) C_app_empty_colors;
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
    io_seproxyhal_display_default(element);
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

const bagl_element_t screen_onboarding_word_list_elements[] = {
  // erase
  {{BAGL_RECTANGLE                      , 0x00,   0,   0, 128,  32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0}, NULL},

  {{BAGL_LABELINE                       , 0x01,   0,  12, 128,  32, 0, 0, 0        , 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, G_ux.string_buffer },
  {{BAGL_RECTANGLE                      , 0x02,  32,  16,  64,  14, 0, 4, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0}, NULL},
  {{BAGL_LABELINE                       , 0x02,   0,  26, 128,  32, 0, 0, 0        , 0x000000, 0xFFFFFF, BAGL_FONT_OPEN_SANS_EXTRABOLD_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, G_ux.string_buffer },

  // left/rights icons
  {{BAGL_ICON                           , 0x03,   3,  12,   4,   7, 0, 0, 0        , 0xFFFFFF, 0x000000, 0, 0}, (const char*)&C_icon_left },
  {{BAGL_ICON                           , 0x04, 121,  12,   4,   7, 0, 0, 0        , 0xFFFFFF, 0x000000, 0, 0}, (const char*)&C_icon_right },

  // supplementary static entry
  {{BAGL_ICON                           , 0x05,  16,   9,  14,  14, 0, 0, 0        , 0xFFFFFF, 0x000000, 0, 0}, (const char*)&C_icon_back },
  {{BAGL_LABELINE                       , 0x05,  41,  12, 128,  32, 0, 0, 0        , 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_EXTRABOLD_11px, 0  }, "Restart from" },
  {{BAGL_LABELINE                       , 0x06,  41,  26, 128,  32, 0, 0, 0        , 0xFFFFFF, 0x000000, BAGL_FONT_OPEN_SANS_EXTRABOLD_11px, 0  }, G_ux.string_buffer },
};


#endif 
