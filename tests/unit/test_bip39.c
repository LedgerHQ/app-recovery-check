#include <stdint.h>
#include <string.h>

#include "Mocklcx_hmac.h"
#include "Mocklcx_pbkdf2.h"
#include "Mocklcx_sha256.h"
#include "Mockos_seed.h"
#include "Mockos_utils.h"
#include "bip39.h"
#include "fake_crypto.h"
#include "nbgl_use_case.h"
#include "unity.h"

// Keys of the Flex keyboard, in the order of the mask bits
static const char KBD_LETTERS[] = "qwertyuiopasdfghjklzxcvbnm";
// Bit of the 'return' key, always enabled
#define RETURN_KEY_BIT (1 << 28)

// Value returned when no word matches
#define NO_WORD BIP39_WORDLIST_OFFSETS_LENGTH

#define NB_SUGGESTIONS NB_MAX_SUGGESTION_BUTTONS

void setUp(void) {
    Mocklcx_hmac_Init();
    Mocklcx_pbkdf2_Init();
    Mocklcx_sha256_Init();
    Mockos_seed_Init();
    Mockos_utils_Init();
    fake_crypto_setup(REFERENCE_ROOT_KEY);
}

void tearDown(void) {
    Mocklcx_hmac_Verify();
    Mocklcx_pbkdf2_Verify();
    Mocklcx_sha256_Verify();
    Mockos_seed_Verify();
    Mockos_utils_Verify();
    Mocklcx_hmac_Destroy();
    Mocklcx_pbkdf2_Destroy();
    Mocklcx_sha256_Destroy();
    Mockos_seed_Destroy();
    Mockos_utils_Destroy();
}

static unsigned int idx(const char* prefix) {
    return bolos_ux_bip39_get_word_idx_starting_with(
        (const unsigned char*)prefix, strlen(prefix));
}

static unsigned int count(const char* prefix) {
    return bolos_ux_bip39_get_word_count_starting_with(
        (const unsigned char*)prefix, strlen(prefix));
}

static bool check(const char* mnemonic) {
    char buffer[256];
    strncpy(buffer, mnemonic, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    return bolos_ux_mnemonic_check((unsigned char*)buffer, strlen(buffer));
}

// Mask with only the given letters enabled, on top of the 'return' key
static uint32_t mask_enabling(const char* letters) {
    uint32_t enabled = RETURN_KEY_BIT;
    for (const char* c = letters; *c != '\0'; c++) {
        enabled |= 1u << (strchr(KBD_LETTERS, *c) - KBD_LETTERS);
    }
    return ~enabled;
}

static void test_idx_strcpy(void) {
    unsigned char word[MAX_WORD_LENGTH + 1];

    TEST_ASSERT_EQUAL(7, bolos_ux_bip39_idx_strcpy(0, word));
    TEST_ASSERT_EQUAL_STRING("abandon", word);
    TEST_ASSERT_EQUAL(6, bolos_ux_bip39_idx_strcpy(1000, word));
    TEST_ASSERT_EQUAL_STRING("laptop", word);
    TEST_ASSERT_EQUAL(3, bolos_ux_bip39_idx_strcpy(2047, word));
    TEST_ASSERT_EQUAL_STRING("zoo", word);
}

static void test_idx_strcpy_nok(void) {
    unsigned char word[MAX_WORD_LENGTH + 1] = "none";

    TEST_ASSERT_EQUAL(0, bolos_ux_bip39_idx_strcpy(2048, word));
    TEST_ASSERT_EQUAL_STRING("none", word);
    TEST_ASSERT_EQUAL(0, bolos_ux_bip39_idx_strcpy(0, NULL));
}

static void test_word_idx_starting_with(void) {
    TEST_ASSERT_EQUAL(0, idx(""));
    TEST_ASSERT_EQUAL(0, idx("ab"));
    TEST_ASSERT_EQUAL(263, idx("can"));
    TEST_ASSERT_EQUAL(1872, idx("tu"));
    TEST_ASSERT_EQUAL(2047, idx("zoo"));
    TEST_ASSERT_EQUAL(NO_WORD, idx("zz"));
    TEST_ASSERT_EQUAL(NO_WORD, idx("abandons"));
}

static void test_word_count_starting_with(void) {
    TEST_ASSERT_EQUAL(2048, count(""));
    TEST_ASSERT_EQUAL(136, count("a"));
    TEST_ASSERT_EQUAL(10, count("ab"));
    // a whole word, also the prefix of 7 others
    TEST_ASSERT_EQUAL(8, count("can"));
    TEST_ASSERT_EQUAL(1, count("zoo"));
    TEST_ASSERT_EQUAL(0, count("zz"));
    TEST_ASSERT_EQUAL(0, count("xyz"));
}

static void test_word_next_letters_starting_with(void) {
    unsigned char letters[27] = {0};

    TEST_ASSERT_EQUAL(6, bolos_ux_bip39_get_word_next_letters_starting_with(
                             (const unsigned char*)"ab", 2, letters));
    TEST_ASSERT_EQUAL_STRING_LEN("ailosu", letters, 6);

    // "can" itself has no next letter, its longer words do
    memset(letters, 0, sizeof(letters));
    TEST_ASSERT_EQUAL(7, bolos_ux_bip39_get_word_next_letters_starting_with(
                             (const unsigned char*)"can", 3, letters));
    TEST_ASSERT_EQUAL_STRING_LEN("acdnovy", letters, 7);

    TEST_ASSERT_EQUAL(0, bolos_ux_bip39_get_word_next_letters_starting_with(
                             (const unsigned char*)"zoo", 3, letters));
    TEST_ASSERT_EQUAL(0, bolos_ux_bip39_get_word_next_letters_starting_with(
                             (const unsigned char*)"zz", 2, letters));
}

static void test_keyboard_mask(void) {
    TEST_ASSERT_EQUAL_HEX32(
        mask_enabling("u"),
        bolos_ux_bip39_get_keyboard_mask((const unsigned char*)"q", 1));
    TEST_ASSERT_EQUAL_HEX32(
        mask_enabling("ailosu"),
        bolos_ux_bip39_get_keyboard_mask((const unsigned char*)"ab", 2));
    // nothing can follow, only 'return' stays enabled
    TEST_ASSERT_EQUAL_HEX32(
        mask_enabling(""),
        bolos_ux_bip39_get_keyboard_mask((const unsigned char*)"zoo", 3));
}

static void test_fill_with_candidates(void) {
    char words[(MAX_WORD_LENGTH + 1) * NB_SUGGESTIONS];
    const char* buttons[NB_SUGGESTIONS] = {0};
    size_t nb_candidates = 0;

    // capped to the 8 buttons of Flex, the real count reported aside
    TEST_ASSERT_EQUAL(8, NB_SUGGESTIONS);
    TEST_ASSERT_EQUAL(NB_SUGGESTIONS, bolos_ux_bip39_fill_with_candidates(
                                          (const unsigned char*)"ab", 2, words,
                                          buttons, &nb_candidates));
    TEST_ASSERT_EQUAL(10, nb_candidates);
    TEST_ASSERT_EQUAL_STRING("abandon", buttons[0]);
    TEST_ASSERT_EQUAL_STRING("ability", buttons[1]);
    TEST_ASSERT_EQUAL_STRING("abstract", buttons[7]);

    // a whole word comes first, before the longer ones
    memset(buttons, 0, sizeof(buttons));
    TEST_ASSERT_EQUAL(
        8, bolos_ux_bip39_fill_with_candidates((const unsigned char*)"can", 3,
                                               words, buttons, &nb_candidates));
    TEST_ASSERT_EQUAL(8, nb_candidates);
    TEST_ASSERT_EQUAL_STRING("can", buttons[0]);
    TEST_ASSERT_EQUAL_STRING("canyon", buttons[7]);

    memset(buttons, 0, sizeof(buttons));
    TEST_ASSERT_EQUAL(
        1, bolos_ux_bip39_fill_with_candidates((const unsigned char*)"zoo", 3,
                                               words, buttons, NULL));
    TEST_ASSERT_EQUAL_STRING("zoo", buttons[0]);
    TEST_ASSERT_NULL(buttons[1]);
}

static void test_fill_with_candidates_none(void) {
    char words[(MAX_WORD_LENGTH + 1) * NB_SUGGESTIONS];
    const char* buttons[NB_SUGGESTIONS] = {0};
    size_t nb_candidates = 1;

    TEST_ASSERT_EQUAL(
        0, bolos_ux_bip39_fill_with_candidates((const unsigned char*)"zz", 2,
                                               words, buttons, &nb_candidates));
    TEST_ASSERT_EQUAL(0, nb_candidates);
    TEST_ASSERT_NULL(buttons[0]);
}

static void test_mnemonic_check_ok(void) {
    TEST_ASSERT_TRUE(
        check("abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon "
              "abandon about"));
    TEST_ASSERT_TRUE(
        check("abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon "
              "abandon abandon abandon abandon abandon abandon abandon agent"));
    TEST_ASSERT_TRUE(
        check("abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon "
              "abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon "
              "abandon abandon abandon art"));
}

static void test_mnemonic_check_wrong_word_count(void) {
    TEST_ASSERT_FALSE(
        check("abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon "
              "about"));
    TEST_ASSERT_FALSE(check("abandon"));
}

static void test_mnemonic_check_wrong_checksum(void) {
    TEST_ASSERT_FALSE(
        check("abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon "
              "abandon abandon"));
}

static void test_mnemonic_check_unknown_word(void) {
    // not in the list
    TEST_ASSERT_FALSE(
        check("abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon "
              "abandon ledger"));
    // only a prefix of a word
    TEST_ASSERT_FALSE(
        check("abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon "
              "abandon abo"));
    // longer than any word
    TEST_ASSERT_FALSE(
        check("abandon abandon abandon abandon abandon abandon abandon abandon "
              "abandon abandon "
              "abandon abandonment"));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_idx_strcpy);
    RUN_TEST(test_idx_strcpy_nok);
    RUN_TEST(test_word_idx_starting_with);
    RUN_TEST(test_word_count_starting_with);
    RUN_TEST(test_word_next_letters_starting_with);
    RUN_TEST(test_keyboard_mask);
    RUN_TEST(test_fill_with_candidates);
    RUN_TEST(test_fill_with_candidates_none);
    RUN_TEST(test_mnemonic_check_ok);
    RUN_TEST(test_mnemonic_check_wrong_word_count);
    RUN_TEST(test_mnemonic_check_wrong_checksum);
    RUN_TEST(test_mnemonic_check_unknown_word);
    return UNITY_END();
}
