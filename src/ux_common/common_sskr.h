
#pragma once

// SSKR helpers
#include "onboarding_seed_rom_variables.h"

// Combine hex value SSKR shares into seed
void bolos_ux_sskr_hex_to_seed(unsigned char *mnemonic_hex,
                               unsigned int mnemonic_len,
                               unsigned int sskr_shares_count,
                               unsigned char *words_buffer,
                               unsigned int *words_buffer_length,
                               unsigned char *seed);

// convert seed from BIP39 to SSKR
unsigned int bolos_ux_bip39_to_sskr_convert(unsigned char *bip39_words_buffer,
                                            unsigned int bip39_words_buffer_length,
                                            unsigned int bip39_onboarding_kind,
                                            unsigned int *sskr_group_descriptor,
                                            uint8_t *sskr_share_count,
                                            unsigned char *sskr_words_buffer,
                                            unsigned int *sskr_words_buffer_length);

unsigned int bolos_ux_sskr_hex_check(unsigned char *mnemonic_hex,
                                     unsigned int mnemonic_length,
                                     unsigned int sskr_share_count);

unsigned int bolos_ux_sskr_get_word_idx_starting_with(unsigned char *prefix,
                                                      unsigned int prefixlength);
unsigned int bolos_ux_sskr_idx_strcpy(unsigned int index, unsigned char *buffer);
unsigned int bolos_ux_sskr_get_word_count_starting_with(unsigned char *prefix,
                                                        unsigned int prefixlength);
unsigned int bolos_ux_sskr_get_word_next_letters_starting_with(unsigned char *prefix,
                                                               unsigned int prefixlength,
                                                               unsigned char *next_letters_buffer);
