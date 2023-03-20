
#pragma once

// SSKR helpers
#include "onboarding_seed_rom_variables.h"

// convert seed from BIP39 to SSKR
unsigned int bolos_ux_bip39_to_sskr_convert(unsigned char *bip39_words_buffer,
                                            unsigned int bip39_words_buffer_length,
                                            unsigned int onboarding_kind,
                                            unsigned int *sskr_group_descriptor,
                                            uint8_t *sskr_share_count,
                                            unsigned char *sskr_words_buffer,
                                            unsigned int *sskr_words_buffer_len);
