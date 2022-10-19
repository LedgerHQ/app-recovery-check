/* @BANNER@ */

#include <os.h>
#include <cx.h>

#include "onboarding_seed_rom_variables.h"
#include "common.h"

#define MAX_WORD_LENGTH 8
#define ALPHABET_LENGTH 27

static const char KBD_LETTERS[ALPHABET_LENGTH] = "qwertyuiopasdfghjklzxcvbnm";

// separated function to lower the stack usage when jumping into pbkdf algorithm
unsigned int bolos_ux_mnemonic_to_seed_hash_length128(unsigned char* mnemonic,
                                                      unsigned int mnemonicLength) {
    if (mnemonicLength > 128) {
        cx_hash_sha512(mnemonic, mnemonicLength, mnemonic, 64);
        // new mnemonic length
        mnemonicLength = 64;
    }
    return mnemonicLength;
}

void bolos_ux_mnemonic_to_seed(unsigned char* mnemonic,
                               unsigned int mnemonicLength,
                               unsigned char* seed) {
    unsigned char passphrase[BIP39_MNEMONIC_LENGTH + 4];
    mnemonicLength = bolos_ux_mnemonic_to_seed_hash_length128(mnemonic, mnemonicLength);

    memcpy(passphrase, BIP39_MNEMONIC, BIP39_MNEMONIC_LENGTH);
    cx_pbkdf2_sha512(mnemonic,
                     mnemonicLength,
                     passphrase,
                     BIP39_MNEMONIC_LENGTH,
                     BIP39_PBKDF2_ROUNDS,
                     seed,
                     64);

    // what happen to the second block for a very short seed ?
}

unsigned int bolos_ux_mnemonic_check(unsigned char* mnemonic, unsigned int mnemonicLength) {
    unsigned int i, n = 0;
    unsigned int bi;
    unsigned char bits[32 + 1];
    unsigned char mask;

    for (i = 0; i < mnemonicLength; i++) {
        if (mnemonic[i] == ' ') {
            n++;
        }
    }
    n++;
    if (n != 12 && n != 18 && n != 24) {
        return 0;
    }
    memset(bits, 0, sizeof(bits));
    i = 0;
    bi = 0;
    while (i < mnemonicLength) {
        unsigned char current_word[10];
        unsigned int current_word_size = 0;
        unsigned int j, k, ki;
        j = 0;
        while (i < mnemonicLength && mnemonic[i] != ' ') {
            if (j >= sizeof(current_word)) {
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
            if ((memcmp(current_word,
                        BIP39_WORDLIST + BIP39_WORDLIST_OFFSETS[k],
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
            return 0;
        }
    }
    if (bi != n * 11) {
        return 0;
    }
    bits[32] = bits[n * 4 / 3];
    cx_hash_sha256(bits, n * 4 / 3, bits, 32);
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
    if ((bits[0] & mask) != (bits[32] & mask)) {
        return 0;
    }

    // alright mnemonic is ok
    return 1;
}

unsigned int bolos_ux_bip39_idx_strcpy(const unsigned int index, unsigned char* buffer) {
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

unsigned int bolos_ux_bip39_get_word_idx_starting_with(const unsigned char* prefix,
                                                       const unsigned int prefixlength) {
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

unsigned int bolos_ux_bip39_get_word_count_starting_with(const unsigned char* prefix,
                                                         const unsigned int prefixlength) {
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
    const unsigned char* prefix,
    const unsigned int prefixlength,
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

#if defined(HAVE_NBGL)
#include <nbgl_layout.h>

// the biggest word of BIP39 list is 8 char (9 with trailing '\0'), and
// the max number of showed suggestions is NB_MAX_SUGGESTION_BUTTONS
static char wordCandidates[(MAX_WORD_LENGTH + 1) * NB_MAX_SUGGESTION_BUTTONS] = {0};

size_t bolos_ux_bip39_fillwith_candidates(
    const unsigned char *startingChars,
    const size_t startingCharsLenght,
    char *outputBuffer[]
    ) {
    const size_t nbMatchingWords = MIN(
        bolos_ux_bip39_get_word_count_starting_with(startingChars, startingCharsLenght),
        NB_MAX_SUGGESTION_BUTTONS
        );
    PRINTF("There are %d possible suggestions\n", nbMatchingWords);
    if (nbMatchingWords == 0) {
        return 0;
    }
    size_t matchingWordIndex = bolos_ux_bip39_get_word_idx_starting_with(
        startingChars,
        startingCharsLenght
        );
    size_t offset = 0;
    for (size_t i = 0; i < nbMatchingWords; i++) {
        unsigned char *wordDest = (unsigned char *)(&wordCandidates + offset);
        const size_t wordSize = bolos_ux_bip39_idx_strcpy(matchingWordIndex, wordDest);
        matchingWordIndex++;
        offset += wordSize + 1;  // + trailing '\0' size
        outputBuffer[i] = (char *)wordDest;
    }
    return nbMatchingWords;
}

uint32_t bolos_ux_bip39_get_keyboard_mask(
    const unsigned char *prefix,
    const unsigned int prefixlength
    ) {
    uint32_t existing_mask = 0;
    unsigned char next_letters[MAX_WORD_LENGTH] = {0};
    const size_t nb_letters = bolos_ux_bip39_get_word_next_letters_starting_with(prefix, prefixlength, next_letters);
    next_letters[nb_letters] = '\0';
    PRINTF("Next letters are in: %s\n", next_letters);
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        for (size_t j = 0; j < nb_letters; j++) {
            if (KBD_LETTERS[i] == next_letters[j]) {
                existing_mask += 1 << i;
            }
        }
    }
    return (-1 ^ existing_mask);
}
#endif
