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

#include "ux_nano.h"

#if defined(TARGET_NANOX) || defined(TARGET_NANOS2)

#ifdef OS_IO_SEPROXYHAL

bolos_ux_context_t G_bolos_ux_context;

unsigned short io_timeout(unsigned short last_timeout) {
    UNUSED(last_timeout);
    // infinite timeout
    return 1;
}

void io_seproxyhal_display(const bagl_element_t* element) {
    io_seproxyhal_display_default((bagl_element_t*) element);
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
            G_bolos_ux_context.hslider3_before = G_bolos_ux_context.hslider3_current;
            G_bolos_ux_context.hslider3_current = G_bolos_ux_context.hslider3_after;
            G_bolos_ux_context.hslider3_after =
                (G_bolos_ux_context.hslider3_after + 1) % G_bolos_ux_context.hslider3_total;
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
            G_bolos_ux_context.hslider3_current = G_bolos_ux_context.hslider3_before;
            G_bolos_ux_context.hslider3_before =
                (G_bolos_ux_context.hslider3_before + G_bolos_ux_context.hslider3_total - 1) %
                G_bolos_ux_context.hslider3_total;
            break;
    }
}

#endif  // OS_IO_SEPROXYHAL

#endif
