
#pragma once

// BIP39 helpers
#include "onboarding_seed_rom_variables.h"

// return 0 if mnemonic is invalid
unsigned int bolos_ux_mnemonic_check(const unsigned char *mnemonic, unsigned int mnemonicLength);

// passphrase will be prefixed with "MNEMONIC" from BIP39, the passphrase content shall start @ 8
void bolos_ux_mnemonic_to_seed(const unsigned char *mnemonic,
                               const unsigned int mnemonicLength,
                               unsigned char *seed /*, unsigned char *workBuffer*/);

unsigned int bolos_ux_bip39_get_word_idx_starting_with(const unsigned char *prefix,
                                                       const unsigned int prefixlength);
unsigned int bolos_ux_bip39_idx_strcpy(const unsigned int index, unsigned char *buffer);
unsigned int bolos_ux_bip39_get_word_count_starting_with(const unsigned char *prefix,
                                                         const unsigned int prefixlength);
unsigned int bolos_ux_bip39_get_word_next_letters_starting_with(const unsigned char *prefix,
                                                                const unsigned int prefixlength,
                                                                unsigned char *next_letters_buffer);

#if defined(HAVE_NBGL)
size_t bolos_ux_bip39_fillwith_candidates(const unsigned char *startingChars,
                                          const size_t startingCharsLenght,
                                          char *outputBuffer[]);
uint32_t bolos_ux_bip39_get_keyboard_mask(const unsigned char *prefix,
                                          const unsigned int prefixlength);
#endif

#ifdef HAVE_ELECTRUM

unsigned int bolos_ux_electrum_new_mnemonic(unsigned int version,
                                            unsigned char *out,
                                            unsigned int outLength);
unsigned int bolos_ux_electrum_mnemonic_check(unsigned int version,
                                              unsigned char *mnemonic,
                                              unsigned int mnemonicLength);

#endif
