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

#include <ux.h>

#ifdef OS_IO_SEPROXYHAL

#ifndef SPRINTF
// avoid typing the size each time
#define SPRINTF(strbuf, ...) snprintf((char*) (strbuf), sizeof(strbuf), __VA_ARGS__)
#endif

#if defined(HAVE_NBGL)
typedef const nbgl_obj_t* (*keyboard_callback_t)(unsigned int event, unsigned int value);
#else
typedef const bagl_element_t* (*keyboard_callback_t)(unsigned int event, unsigned int value);
#endif

void bolos_ux_hslider3_init(unsigned int total_count);
void bolos_ux_hslider3_set_current(unsigned int current);
void bolos_ux_hslider3_next(void);
void bolos_ux_hslider3_previous(void);

// all screens
void screen_onboarding_3_restore_init(void);
void screen_onboarding_4_restore_word_init(unsigned int action);

#endif  // OS_IO_SEPROXYHAL
