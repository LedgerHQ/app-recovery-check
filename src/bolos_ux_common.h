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

#include "bolos_ux.h"

#ifdef OS_IO_SEPROXYHAL

// avoid typing the size each time
#define SPRINTF(strbuf, ...) snprintf(strbuf, sizeof(strbuf), __VA_ARGS__)

#define ONBOARDING_WORD_COMPLETION_MAX_ITEMS 8

#define ARRAYLEN(array) (sizeof(array) / sizeof(array[0]))

extern bolos_ux_context_t G_bolos_ux_context;

extern const unsigned char hex_digits[];


// common code for all screens
// start display of first declared element

// request display of the element (taking care of calling screen displayed
// preprocessors)
void screen_display_element(const bagl_element_t *element);

// all screens
void screen_onboarding_3_restore_init(void);
void screen_onboarding_4_restore_word_init(unsigned int firstWord);
// apply settings @ boot time

typedef unsigned int (*pin_callback_t)(unsigned char *pin_buffer,
                                       unsigned int pin_length);

#define COMMON_KEYBOARD_INDEX_UNCHANGED (-1UL)
void screen_common_keyboard_init(unsigned int stack_slot,
                                 unsigned int current_element,
                                 unsigned int nb_elements,
                                 keyboard_callback_t callback);

#endif // OS_IO_SEPROXYHAL
