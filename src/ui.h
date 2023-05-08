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
#include <os_io_seproxyhal.h>
#include <string.h>
#include <cx.h>

#include "glyphs.h"
#include "ux_nano.h"

#define memzero(...) explicit_bzero(__VA_ARGS__)

#if defined(TARGET_NANOS)
#define ARRAYLEN(array) (sizeof(array) / sizeof(array[0]))
#define BIP39_ICON      C_bip39_nanos
#define SSKR_ICON       C_sskr_nanos
#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)
#define BIP39_ICON C_bip39_nanox
#define SSKR_ICON  C_sskr_nanox
#endif

void ui_idle_init(void);
