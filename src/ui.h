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

#if defined(TARGET_NANOS)
#include "ux_nanos.h"
#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)
#include "ux_nanox.h"
#elif defined(TARGET_FATSTACKS)
#include "ux_stax.h"
#endif

void ui_idle_init(void);
