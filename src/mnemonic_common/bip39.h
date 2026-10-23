#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// BIP39 helpers
#include "constants.h"
#include "seed_rom_variables.h"

// whether the mnemonic is invalid or not
bool bolos_ux_mnemonic_check(unsigned char *mnemonic, unsigned int mnemonicLength);

// passphrase (may be NULL) is prefixed with "mnemonic" to build the BIP39 salt
void bolos_ux_mnemonic_to_seed(unsigned char *mnemonic,
                               unsigned int mnemonicLength,
                               const unsigned char *passphrase,
                               unsigned int passphraseLength,
                               unsigned char *seed);

unsigned int bolos_ux_bip39_get_word_idx_starting_with(const unsigned char *prefix,
                                                       const unsigned int prefixlength);
unsigned int bolos_ux_bip39_idx_strcpy(const unsigned int index, unsigned char *buffer);
unsigned int bolos_ux_bip39_get_word_count_starting_with(const unsigned char *prefix,
                                                         const unsigned int prefixLength);
unsigned int bolos_ux_bip39_get_word_next_letters_starting_with(const unsigned char *prefix,
                                                                const unsigned int prefixLength,
                                                                unsigned char *next_letters_buffer);
bool compare_recovery_phrase(uint8_t *buffer,
                             size_t buffer_size,
                             const uint8_t *passphrase,
                             size_t passphrase_length);

// Fills the suggestion buffers with up to NB_MAX_SUGGESTION_BUTTONS words matching the given
// prefix and returns that (capped) number. When not NULL, *nbCandidates is set to the total number
// of matching words (uncapped), which the Nano keyboard uses to decide when the whole list fits.
size_t bolos_ux_bip39_fill_with_candidates(const unsigned char *startingChars,
                                           const size_t startingCharsLength,
                                           char wordCandidatesBuffer[],
                                           const char *wordIndexorBuffer[],
                                           size_t *nbCandidates);
uint32_t bolos_ux_bip39_get_keyboard_mask(const unsigned char *prefix,
                                          const unsigned int prefixLength);
