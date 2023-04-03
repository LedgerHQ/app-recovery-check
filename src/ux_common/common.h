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

#include <os.h>
#include <cx.h>
#include <ux.h>

#include <os_io_seproxyhal.h>
#include <string.h>

#include "glyphs.h"

#ifdef OS_IO_SEPROXYHAL

#ifndef SPRINTF
// avoid typing the size each time
#define SPRINTF(strbuf, ...) snprintf((char*) (strbuf), sizeof(strbuf), __VA_ARGS__)
#endif

#define ONBOARDING_WORD_COMPLETION_MAX_ITEMS 8
#define BOLOS_UX_HASH_LENGTH                 4  // as on the blue

#define KEYBOARD_ITEM_VALIDATED \
    1  // callback is called with the entered item index, tmp_element is precharged with element to
       // be displayed and using the common string buffer as string parameter
#define KEYBOARD_RENDER_ITEM \
    2  // callback is called with the element index, tmp_element is precharged with element to be
       // displayed and using the common string buffer as string parameter
#define KEYBOARD_RENDER_WORD \
    3  // callback is called with a -1 when requesting complete word, or the char index else,
       // returnin 0 implies no char is to be displayed
typedef const bagl_element_t* (*keyboard_callback_t)(unsigned int event, unsigned int value);

void bolos_ux_hslider3_init(unsigned int total_count);
void bolos_ux_hslider3_set_current(unsigned int current);
void bolos_ux_hslider3_next(void);
void bolos_ux_hslider3_previous(void);

// all screens
void screen_onboarding_1_restore_init(void);
void screen_onboarding_2_restore_init(void);
#define RESTORE_WORD_ACTION_REENTER_WORD 0
#define RESTORE_WORD_ACTION_FIRST_WORD   1
void screen_onboarding_restore_word_init(unsigned int action);

#define COMMON_KEYBOARD_INDEX_UNCHANGED (-1UL)

#endif  // OS_IO_SEPROXYHAL
