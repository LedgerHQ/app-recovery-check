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

#if defined(HAVE_BOLOS_UX) && defined(TARGET_FATSTACKS)

#define MAX_MNEMONIC_LENGTH (MNEMONIC_SIZE_24 * (MAX_WORD_LENGTH+1))

typedef struct buffer {
    // the mnemonic passphrase, built over time
    char buffer[MAX_MNEMONIC_LENGTH];
    // current length of the mnemonic passphrase
    size_t length;
    // index of the current word ((size_t)-1 mean there is no word currently)
    size_t current_word_index;
    // array of every stored word lengths (used for removing them if needed)
    size_t word_lengths[MNEMONIC_SIZE_24];
    // expected number of word in the final mnemonic (12 or 18 or 24)
    size_t final_size;
} buffer_t;

/*
 * Sets how many words are expected in the mnemonic passphrase
 */
void set_mnemonic_final_size(const size_t size);

/*
 * Returns how many words are expected in the mnemonic passphrase
 */
size_t get_mnemonic_final_size(void);

/*
 * Returns how many words are currently stored in the mnemonic
 */
size_t get_current_word_number(void);

/*
 * Check if the current number of words in the mnemonic fits the expected number of words
 */
bool is_mnemonic_complete(void);

/*
 * Check if the currently stored mnemonic generates the same seed as the current device's one
 */
bool check_mnemonic(void);

/*
 * Erase all information and reset the indexes
 */
void reset_mnemonic(void);

/*
 * Remove the latest word from the passphrase, returns true if there was at least one to remove, else
 * false (there was no word)
 */
bool remove_word_from_mnemonic(void);

/*
 * Adds a word in the passphrase, returns how many words are stored in the mnemonic
 */
size_t add_word_in_mnemonic(const char* const buffer, const size_t size);

#endif  // HAVE_BOLOS_UX && TARGET_NANOS
