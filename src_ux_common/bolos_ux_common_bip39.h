
#ifndef COMMON_BIP39_H
#define COMMON_BIP39_H

// BIP39 helpers
#include "bolos_ux_onboarding_seed_rom_variables.h"

void bolos_ux_pbkdf2(unsigned char* password, unsigned int passwordlen, unsigned char* salt, unsigned int saltlen, unsigned int iterations, unsigned char* out, unsigned int outLength);
unsigned char bolos_ux_get_random_bip39_word(unsigned char *word);
// return 0 if mnemonic is invalid
unsigned int bolos_ux_mnemonic_check(unsigned char *mnemonic, unsigned int mnemonicLength);
unsigned char bolos_ux_word_check(unsigned char *word, unsigned int wordLength);

unsigned int bolos_ux_get_word_ptr(unsigned char ** word, unsigned int max_length, unsigned int word_index);

// passphrase will be prefixed with "MNEMONIC" from BIP39, the passphrase content shall start @ 8
void bolos_ux_mnemonic_to_seed(unsigned char *mnemonic, unsigned int mnemonicLength, unsigned char *seed/*, unsigned char *workBuffer*/);
unsigned int bolos_ux_mnemonic_indexes_to_words(unsigned char *indexes, unsigned char *words);
unsigned int bolos_ux_mnemonic_from_data(unsigned char *in, unsigned int inLength, unsigned char *out, unsigned int outLength);


unsigned int bolos_ux_bip39_get_word_idx_starting_with(unsigned char* prefix, unsigned int prefixlength);
unsigned int bolos_ux_bip39_idx_strcpy(unsigned int index, unsigned char* buffer);
unsigned int bolos_ux_bip39_idx_startswith(unsigned int idx, unsigned char* prefix, unsigned int prefixlength);
unsigned int bolos_ux_bip39_get_word_count_starting_with(unsigned char* prefix, unsigned int prefixlength);
unsigned int bolos_ux_bip39_get_word_next_letters_starting_with(unsigned char* prefix, unsigned int prefixlength, unsigned char* next_letters_buffer);


#ifdef HAVE_ELECTRUM

unsigned int bolos_ux_electrum_new_mnemonic(unsigned int version, unsigned char *out, unsigned int outLength);
unsigned int bolos_ux_electrum_mnemonic_check(unsigned int version, unsigned char *mnemonic, unsigned int mnemonicLength);

#endif

#endif // COMMON_BIP39
