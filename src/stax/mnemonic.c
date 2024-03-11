#include <os.h>
#include <string.h>

#include "../ux_common/common_bip39.h"
#include "./mnemonic.h"

#if defined(SCREEN_SIZE_WALLET)

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

static buffer_t mnemonic = {0};

size_t mnemonic_shrink(const size_t size) {
    if (size == 0 || size > mnemonic.length) {
        // shrink all
        mnemonic.length = 0;
    } else {
        mnemonic.length -= size;
    }
    memset(&mnemonic.buffer[mnemonic.length], 0, MAX_MNEMONIC_LENGTH - mnemonic.length);
    return mnemonic.length;
}

void set_mnemonic_final_size(const size_t size) {
    mnemonic.final_size = size;
}

size_t get_mnemonic_final_size() {
    return mnemonic.final_size;
}

size_t get_current_word_number() {
    return mnemonic.current_word_index + 1;
}

void reset_mnemonic() {
    memset(&mnemonic, 0, sizeof(mnemonic));
    mnemonic.current_word_index = (size_t) -1;
}

bool remove_word_from_mnemonic() {
    PRINTF("Removing a word, currently there is '%ld' of them\n", mnemonic.current_word_index + 1);
    if (mnemonic.current_word_index == (size_t) -1) {
        return false;
    }
    const size_t current_length = mnemonic.word_lengths[mnemonic.current_word_index];
    mnemonic.current_word_index--;
    // removing previous word from mnemonic buffer (+ 1 blank space)
    mnemonic_shrink(current_length + 1);
    PRINTF("Number of remaining words in the mnemonic: '%ld'\n", mnemonic.current_word_index + 1);
    return true;
}

size_t add_word_in_mnemonic(const char* const buffer, const size_t size) {
    if (mnemonic.current_word_index != (size_t) -1) {
        // adding an extra white space ' ' between words
        mnemonic.buffer[mnemonic.length++] = ' ';
        mnemonic.buffer[mnemonic.length] = '\0';
    }
    memcpy(&mnemonic.buffer[0] + mnemonic.length, buffer, size);
    mnemonic.length += size;
    mnemonic.current_word_index++;
    mnemonic.word_lengths[mnemonic.current_word_index] = size;
    PRINTF("Number of words in the mnemonic: '%ld'\n", get_current_word_number());
    PRINTF("Current mnemonic: '%s'\n", &mnemonic.buffer[0]);
    return get_current_word_number();
}

bool is_mnemonic_complete() {
    return (mnemonic.final_size == 0
                ? false
                : (mnemonic.current_word_index + 1) >= get_mnemonic_final_size());
}

bool check_mnemonic() {
    if (!is_mnemonic_complete()) {
        return false;
    }
    PRINTF("Checking the following mnemonic: '%s' (size %ld)\n",
           &mnemonic.buffer[0],
           mnemonic.length);
    const bool result =
        bolos_ux_mnemonic_check((unsigned char*) &mnemonic.buffer[0], mnemonic.length);
    // clearing the mnemonic ASAP
    reset_mnemonic();
    return result;
}

#if defined(TEST)
char* get_mnemonic() {
    return mnemonic.buffer;
}
#endif

#endif
