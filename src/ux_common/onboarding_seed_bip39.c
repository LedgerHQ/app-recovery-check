/* @BANNER@ */

#include <os.h>
#include <cx.h>

#include "onboarding_seed_rom_variables.h"
#include "common.h"

// separated function to lower the stack usage when jumping into pbkdf algorithm
unsigned int bolos_ux_bip39_mnemonic_to_seed_hash_length128(unsigned char* mnemonic,
                                                            unsigned int mnemonicLength) {
    if (mnemonicLength > 128) {
        cx_hash_sha512(mnemonic, mnemonicLength, mnemonic, 64);
        // new mnemonic length
        mnemonicLength = 64;
    }
    return mnemonicLength;
}

void bolos_ux_bip39_mnemonic_to_seed(unsigned char* mnemonic,
                                     unsigned int mnemonicLength,
                                     unsigned char* seed) {
    // Need to keep BIP39 mnemonic in case we want to generate SSKR from it
    // It will be zeroed later if not needed
    unsigned char mnemonic_hash[mnemonicLength];
    memcpy(mnemonic_hash, mnemonic, mnemonicLength);
    unsigned char passphrase[BIP39_MNEMONIC_LENGTH + 4];
    mnemonicLength = bolos_ux_bip39_mnemonic_to_seed_hash_length128(mnemonic_hash, mnemonicLength);
    memcpy(passphrase, BIP39_MNEMONIC, BIP39_MNEMONIC_LENGTH);
    cx_pbkdf2_sha512(mnemonic_hash,
                     mnemonicLength,
                     passphrase,
                     BIP39_MNEMONIC_LENGTH,
                     BIP39_PBKDF2_ROUNDS,
                     seed,
                     64);
    memset(mnemonic_hash, 0, mnemonicLength);
    PRINTF("BIP39 seed:\n %.*H\n", 64, seed);
}

unsigned int bolos_ux_bip39_mnemonic_decode(unsigned char* mnemonic,
                                            unsigned int mnemonicLength,
                                            unsigned char* bits,
                                            unsigned int bitslength) {
    unsigned int i, n = 0;
    unsigned int bi;
    unsigned char buffer[32];
    unsigned char mask;

    PRINTF("BIP39 mnemonic phrase:\n %.*s\n", mnemonicLength, mnemonic);

    for (i = 0; i < mnemonicLength; i++) {
        if (mnemonic[i] == ' ') {
            n++;
        }
    }
    n++;
    if (n != 12 && n != 18 && n != 24) {
        return 0;
    }
    memset(bits, 0, bitslength);
    i = 0;
    bi = 0;
    while (i < mnemonicLength) {
        unsigned char current_word[10];
        unsigned int current_word_size = 0;
        unsigned int j, k, ki;
        j = 0;
        while (i < mnemonicLength && mnemonic[i] != ' ') {
            if (j >= sizeof(current_word)) {
                memset(bits, 0, bitslength);
                return 0;
            }
            current_word[j] = mnemonic[i];
            current_word_size = j;
            i++;
            j++;
        }
        if (i < mnemonicLength) {
            i++;
        }
        current_word_size++;
        for (k = 0; k < BIP39_WORDLIST_OFFSETS_LENGTH - 1; k++) {
            if ((os_secure_memcmp(current_word,
                                  (void*) (BIP39_WORDLIST + BIP39_WORDLIST_OFFSETS[k]),
                                  current_word_size) == 0) &&
                ((unsigned int) (BIP39_WORDLIST_OFFSETS[k + 1] - BIP39_WORDLIST_OFFSETS[k]) ==
                 current_word_size)) {
                for (ki = 0; ki < 11; ki++) {
                    if (k & (1 << (10 - ki))) {
                        bits[bi / 8] |= 1 << (7 - (bi % 8));
                    }
                    bi++;
                }
                break;
            }
        }
        if (k == (unsigned int) (BIP39_WORDLIST_OFFSETS_LENGTH - 1)) {
            memset(bits, 0, bitslength);
            return 0;
        }
    }
    if (bi != n * 11) {
        memset(bits, 0, bitslength);
        return 0;
    }

    cx_hash_sha256(bits, n * 4 / 3, buffer, 32);
    switch (n) {
        case 12:
            mask = 0xF0;
            break;
        case 18:
            mask = 0xFC;
            break;
        default:
            mask = 0xFF;
            break;
    }
    if ((buffer[0] & mask) != (bits[n * 4 / 3] & mask)) {
        memset(bits, 0, bitslength);
        return 0;
    }

    // alright mnemonic is ok
    PRINTF("BIP39 mnemonic decoded in hex:\n %.*H\n", bitslength, bits);
    return 1;
}

unsigned int bolos_ux_bip39_mnemonic_check(unsigned char* mnemonic, unsigned int mnemonicLength) {
    unsigned char bits[32 + 1];

    if (bolos_ux_bip39_mnemonic_decode(mnemonic, mnemonicLength, bits, 32 + 1) != 1) {
        memset(bits, 0, 32 + 1);
        return 0;
    }

    // alright mnemonic is ok
    return 1;
}

unsigned int bolos_ux_bip39_idx_strcpy(unsigned int index, unsigned char* buffer) {
    if (index < BIP39_WORDLIST_OFFSETS_LENGTH - 1 && buffer) {
        size_t wordLength = BIP39_WORDLIST_OFFSETS[index + 1] - BIP39_WORDLIST_OFFSETS[index];
        memcpy(buffer, BIP39_WORDLIST + BIP39_WORDLIST_OFFSETS[index], wordLength);
        buffer[wordLength] = 0;  // EOS
        return wordLength;
    }
    // no word at that index
    // buffer[0] = 0; // EOS
    return 0;
}

unsigned int bolos_ux_bip39_get_word_idx_starting_with(unsigned char* prefix,
                                                       unsigned int prefixlength) {
    unsigned int i;
    for (i = 0; i < BIP39_WORDLIST_OFFSETS_LENGTH - 1; i++) {
        unsigned int j = 0;
        while (j < (unsigned int) (BIP39_WORDLIST_OFFSETS[i + 1] - BIP39_WORDLIST_OFFSETS[i]) &&
               j < prefixlength && BIP39_WORDLIST[BIP39_WORDLIST_OFFSETS[i] + j] == prefix[j]) {
            j++;
        }
        if (j == prefixlength) {
            return i;
        }
    }
    // no match, sry
    return BIP39_WORDLIST_OFFSETS_LENGTH;
}

unsigned int bolos_ux_bip39_get_word_count_starting_with(unsigned char* prefix,
                                                         unsigned int prefixlength) {
    unsigned int i;
    unsigned int count = 0;
    for (i = 0; i < BIP39_WORDLIST_OFFSETS_LENGTH - 1; i++) {
        unsigned int j = 0;
        while (j < (unsigned int) (BIP39_WORDLIST_OFFSETS[i + 1] - BIP39_WORDLIST_OFFSETS[i]) &&
               j < prefixlength && BIP39_WORDLIST[BIP39_WORDLIST_OFFSETS[i] + j] == prefix[j]) {
            j++;
        }
        if (j == prefixlength) {
            count++;
        }
        // don't seek till the end, abort when the prefix is not matched anymore
        else if (count > 0) {
            break;
        }
    }
    // return number of matched word starting with the given prefix
    return count;
}

// allocate at most 26 letters for next possibilities
// algorithm considers the bip39 words are alphabetically ordered in the wordlist
unsigned int bolos_ux_bip39_get_word_next_letters_starting_with(
    unsigned char* prefix,
    unsigned int prefixlength,
    unsigned char* next_letters_buffer) {
    unsigned int i;
    unsigned int letter_count = 0;
    for (i = 0; i < BIP39_WORDLIST_OFFSETS_LENGTH - 1; i++) {
        unsigned int j = 0;
        while (j < (unsigned int) (BIP39_WORDLIST_OFFSETS[i + 1] - BIP39_WORDLIST_OFFSETS[i]) &&
               j < prefixlength && BIP39_WORDLIST[BIP39_WORDLIST_OFFSETS[i] + j] == prefix[j]) {
            j++;
        }
        if (j == prefixlength) {
            if (j < (unsigned int) (BIP39_WORDLIST_OFFSETS[i + 1] - BIP39_WORDLIST_OFFSETS[i])) {
                // j is inc during previous loop, don't touch it
                unsigned char next_letter = BIP39_WORDLIST[BIP39_WORDLIST_OFFSETS[i] + j];
                // add the first next_letter inconditionnally
                if (letter_count == 0) {
                    next_letters_buffer[0] = next_letter;
                    letter_count = 1;
                }
                // the next_letter is different
                else if (next_letters_buffer[0] != next_letter) {
                    next_letters_buffer++;
                    next_letters_buffer[0] = next_letter;
                    letter_count++;
                }
            }
        }
        // don't seek till the end, abort when the prefix is not matched anymore
        else if (letter_count > 0) {
            break;
        }
    }
    // return number of matched word starting with the given prefix
    return letter_count;
}
