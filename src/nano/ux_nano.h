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

#pragma once

#include "ux_common/common.h"

#if (defined(TARGET_NANOS) || defined(TARGET_NANOX) || defined(TARGET_NANOS2))

// bolos ux context (not mandatory if redesigning a bolos ux)
typedef struct bolos_ux_context {
    unsigned int onboarding_kind;

#ifdef HAVE_ELECTRUM
    unsigned int onboarding_algorithm;
#endif

    unsigned int onboarding_step;
    unsigned int onboarding_index;
    unsigned int onboarding_words_checked;

    unsigned int words_buffer_length;

    // 128 of words (215 => hashed to 64, or 128) + HMAC_LENGTH*2 = 256
#define WORDS_BUFFER_MAX_SIZE_B 257
    char words_buffer[WORDS_BUFFER_MAX_SIZE_B];

#if defined(TARGET_NANOS)
    // after an int to make sure it's aligned
    char string_buffer[MAX(
        64,
        sizeof(bagl_icon_details_t) + BOLOS_APP_ICON_SIZE_B - 1)];  // to store the seed wholly
#else
    // label line for common PIN and common keyboard screen (displayed over the entry)
    const char* common_label;
#endif  // defined(TARGET_NANOS)

    // slider management / menu list management
    unsigned int hslider3_before;
    unsigned int hslider3_current;
    unsigned int hslider3_after;
    unsigned int hslider3_total;

    keyboard_callback_t keyboard_callback;
    // for CheckSeed app only
    uint8_t processing;

} bolos_ux_context_t;

extern bolos_ux_context_t G_bolos_ux_context;

// update before, current, after index for horizontal slider with 3 positions
// slider distinguish handling from the data, to be more generic :)
#define BOLOS_UX_HSLIDER3_NONE (-1UL)

void screen_common_keyboard_init(unsigned int stack_slot,
                                 unsigned int current_element,
                                 unsigned int nb_elements,
                                 keyboard_callback_t callback);

#include "ux_common/common_bip39.h"

#if defined(TARGET_NANOS)
extern const bagl_element_t screen_onboarding_word_list_elements[9];
void compare_recovery_phrase(void);
#else
// to be included into all flow that needs to go back to the dashboard
extern const ux_flow_step_t ux_ob_goto_dashboard_step;
#endif  // defined(TARGET_NANOS)

#endif  // (TARGET_NANOS || TARGET_NANOX || TARGET_NANOS2)
