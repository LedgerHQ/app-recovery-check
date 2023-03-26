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

#if defined(HAVE_BOLOS_UX) && (defined(TARGET_NANOX) || defined(TARGET_NANOS2))

enum e_state {
    STATIC_SCREEN,
    DYNAMIC_SCREEN,
};

enum e_onboarding_type {
    BIP39_ONBOARDING,
    SSKR_ONBOARDING,
};

// bolos ux context (not mandatory if redesigning a bolos ux)
typedef struct bolos_ux_context {
#define BOLOS_UX_ONBOARDING_NEW        1
#define BOLOS_UX_ONBOARDING_NEW_12     12
#define BOLOS_UX_ONBOARDING_NEW_18     18
#define BOLOS_UX_ONBOARDING_NEW_24     24
#define BOLOS_UX_ONBOARDING_RESTORE    2
#define BOLOS_UX_ONBOARDING_RESTORE_12 12
#define BOLOS_UX_ONBOARDING_RESTORE_18 18
#define BOLOS_UX_ONBOARDING_RESTORE_24 24
    unsigned int bip39_onboarding_kind;

#ifdef HAVE_ELECTRUM
    unsigned int onboarding_algorithm;
#endif

    unsigned int onboarding_step;
    unsigned int onboarding_index;
    unsigned int onboarding_words_checked;

    unsigned int words_buffer_length;
    // after an int to make sure it's aligned
    char string_buffer[MAX(64, sizeof(bagl_icon_details_t) + BOLOS_APP_ICON_SIZE_B - 1)];

    // 128 of words (215 => hashed to 64, or 128) + HMAC_LENGTH*2 = 256
#define WORDS_BUFFER_MAX_SIZE_B 257
    char words_buffer[WORDS_BUFFER_MAX_SIZE_B];
#define MAX_PIN_LENGTH 8
#define MIN_PIN_LENGTH 4
    char pin_digit_buffer;  // digit to be displayed

    appmain_t flow_end_callback;

    // label line for common PIN and common keyboard screen (displayed over the entry)
    const char* common_label;

    // slider management / menu list management
    unsigned int hslider3_before;
    unsigned int hslider3_current;
    unsigned int hslider3_after;
    unsigned int hslider3_total;

    keyboard_callback_t keyboard_callback;

    unsigned int overlay_refresh;
    unsigned int battery_percentage;
    unsigned int status_batt_level;
    unsigned int status_flags;
    unsigned int batt_low_displayed;
    unsigned int batt_critical_displayed;

#define BATTERY_FULL_CHARGE_MV          4200  // 100%
#define BATTERY_SUFFICIENT_CHARGE_MV    3840  //  40%
#define BATTERY_LOW_LEVEL_MV            3750  //  25%
#define BATTERY_CRITICAL_LEVEL_MV       3460  //  10%
#define BATTERY_AUTO_POWER_OFF_LEVEL_MV 3200  //   0%

#define BATTERY_FULL_CHARGE_PERCENT          95
#define BATTERY_SUFFICIENT_CHARGE_PERCENT    40
#define BATTERY_LOW_LEVEL_PERCENT            25
#define BATTERY_CRITICAL_LEVEL_PERCENT       10
#define BATTERY_AUTO_POWER_OFF_LEVEL_PERCENT 2

    // detect stack/global variable overlap
    // have a zero byte to avoid buffer overflow from strings in the ux (we never know)
#define CANARY_MAGIC 0x7600E9AB
    unsigned int canary;

    // for CheckSeed app only
    uint8_t processing;

    // State of the dynamic display.
    enum e_state current_state;

    // Type of onboarding we are performing (BIP39 or SSKR)
    enum e_onboarding_type onboarding_type;

    uint8_t sskr_share_count;
    uint8_t sskr_share_index;
    unsigned int sskr_group_descriptor[1][2];
    unsigned int sskr_words_buffer_len;
    char sskr_words_buffer[];
} bolos_ux_context_t;

extern bolos_ux_context_t G_bolos_ux_context;

// update before, current, after index for horizontal slider with 3 positions
// slider distinguish handling from the data, to be more generic :)
#define BOLOS_UX_HSLIDER3_NONE (-1UL)

void screen_common_keyboard_init(unsigned int stack_slot,
                                 unsigned int current_element,
                                 unsigned int nb_elements,
                                 keyboard_callback_t callback);

void generate_sskr(void);

#include "ux_common/common_bip39.h"
#include "ux_common/common_sskr.h"

// to be included into all flow that needs to go back to the dashboard
extern const ux_flow_step_t ux_ob_goto_dashboard_step;

#endif  // HAVE_BOLOS_UX_H && (TARGET_NANOX || TARGET_NANOS2)
