#include <stdint.h>
#include <string.h>

#include "Mocklcx_hmac.h"
#include "Mocklcx_pbkdf2.h"
#include "Mocklcx_sha256.h"
#include "Mockos_seed.h"
#include "Mockos_utils.h"
#include "fake_crypto.h"
#include "mnemonic.h"
#include "unity.h"

// Default Speculos mnemonic
static const char* const SPECULOS_MNEMONIC[] = {
    "glory", "promote",  "mansion", "idle",    "axis",  "finger",
    "extra", "february", "uncover", "one",     "trip",  "resource",
    "lawn",  "turtle",   "enact",   "monster", "seven", "myth",
    "punch", "hobby",    "comfort", "wild",    "raise", "skin"};

void setUp(void) {
    // resets the whole buffer with initial values
    reset_mnemonic();
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

static void add_words(const char* const* words, size_t nb_words) {
    set_mnemonic_final_size(nb_words);
    for (size_t i = 0; i < nb_words; i++) {
        TEST_ASSERT_EQUAL(i + 1,
                          add_word_in_mnemonic(words[i], strlen(words[i])));
    }
}

static void add_speculos_mnemonic(void) {
    add_words(SPECULOS_MNEMONIC,
              sizeof(SPECULOS_MNEMONIC) / sizeof(SPECULOS_MNEMONIC[0]));
}

static void test_mnemonic_init(void) {
    TEST_ASSERT_EQUAL(0, get_mnemonic_final_size());
    TEST_ASSERT_EQUAL(0, get_current_word_number());
    TEST_ASSERT_EQUAL_STRING("", get_mnemonic());
    TEST_ASSERT_FALSE(is_mnemonic_complete());
    TEST_ASSERT_FALSE(check_mnemonic());
}

static void test_mnemonic_final_size(void) {
    const size_t test = 9;
    set_mnemonic_final_size(test);
    TEST_ASSERT_EQUAL(test, get_mnemonic_final_size());
}

static void test_add_word_in_mnemonic(void) {
    const char* word = "hello";
    TEST_ASSERT_EQUAL(0, get_current_word_number());
    TEST_ASSERT_EQUAL_STRING("", get_mnemonic());

    TEST_ASSERT_EQUAL(1, add_word_in_mnemonic(word, 5));
    TEST_ASSERT_EQUAL(1, get_current_word_number());
    TEST_ASSERT_EQUAL_STRING(word, get_mnemonic());
}

static void test_remove_word_from_mnemonic_none(void) {
    TEST_ASSERT_FALSE(remove_word_from_mnemonic());
}

static void test_remove_word_from_mnemonic(void) {
    const char* word = "list";
    TEST_ASSERT_EQUAL(0, get_current_word_number());
    TEST_ASSERT_EQUAL_STRING("", get_mnemonic());

    add_word_in_mnemonic(word, 4);
    TEST_ASSERT_EQUAL(1, get_current_word_number());
    TEST_ASSERT_EQUAL_STRING(word, get_mnemonic());

    TEST_ASSERT_TRUE(remove_word_from_mnemonic());
    TEST_ASSERT_EQUAL(0, get_current_word_number());
    TEST_ASSERT_EQUAL_STRING("", get_mnemonic());
}

static void test_is_mnemonic_complete(void) {
    set_mnemonic_final_size(2);
    TEST_ASSERT_FALSE(is_mnemonic_complete());

    const char* word = "list";
    TEST_ASSERT_EQUAL(1, add_word_in_mnemonic(word, 4));
    TEST_ASSERT_FALSE(is_mnemonic_complete());

    TEST_ASSERT_EQUAL(2, add_word_in_mnemonic(word, 4));
    TEST_ASSERT_TRUE(is_mnemonic_complete());

    set_mnemonic_final_size(3);
    TEST_ASSERT_FALSE(is_mnemonic_complete());

    set_mnemonic_final_size(1);
    TEST_ASSERT_TRUE(is_mnemonic_complete());

    set_mnemonic_final_size(0);
    TEST_ASSERT_FALSE(is_mnemonic_complete());
}

static void test_check_mnemonic_nok(void) {
    TEST_ASSERT_FALSE(check_mnemonic());

    set_mnemonic_final_size(1);
    TEST_ASSERT_FALSE(check_mnemonic());

    TEST_ASSERT_EQUAL(1, add_word_in_mnemonic("word", 4));
    TEST_ASSERT_EQUAL_STRING("word", get_mnemonic());
    TEST_ASSERT_FALSE(check_mnemonic());
}

static void test_check_mnemonic_nok2(void) {
    // only 12 words of the mnemonic -> fail
    add_words(SPECULOS_MNEMONIC, 12);

    TEST_ASSERT_FALSE(check_mnemonic());
}

static void test_check_mnemonic_ok(void) {
    add_speculos_mnemonic();

    // The fakes model a device holding this very mnemonic, so the comparison
    // against the device root key must succeed.
    TEST_ASSERT_TRUE(check_mnemonic());
    // a successful check clears the buffer as well
    TEST_ASSERT_EQUAL(0, get_current_word_number());
    TEST_ASSERT_EQUAL_STRING("", get_mnemonic());
}

static void test_check_mnemonic_not_device_seed(void) {
    // A valid BIP39 mnemonic, but not the one the device holds: the checksum
    // passes, so the failure can only come from the root key comparison.
    const char* const mnemonic[] = {
        "abandon", "abandon", "abandon", "abandon", "abandon", "abandon",
        "abandon", "abandon", "abandon", "abandon", "abandon", "abandon",
        "abandon", "abandon", "abandon", "abandon", "abandon", "abandon",
        "abandon", "abandon", "abandon", "abandon", "abandon", "art"};
    add_words(mnemonic, sizeof(mnemonic) / sizeof(mnemonic[0]));

    TEST_ASSERT_FALSE(check_mnemonic());
    TEST_ASSERT_EQUAL(0, get_current_word_number());
    TEST_ASSERT_EQUAL_STRING("", get_mnemonic());
}

static void test_check_mnemonic_passphrase_ok(void) {
    fake_crypto_setup(REFERENCE_PASSPHRASE_ROOT_KEY);
    add_speculos_mnemonic();
    TEST_ASSERT_TRUE(set_mnemonic_passphrase(REFERENCE_PASSPHRASE,
                                             strlen(REFERENCE_PASSPHRASE)));

    TEST_ASSERT_TRUE(check_mnemonic());
    // the passphrase is wiped along with the mnemonic
    TEST_ASSERT_EQUAL(0, get_mnemonic_passphrase_length());
    TEST_ASSERT_EQUAL_STRING("", get_mnemonic());
}

static void test_check_mnemonic_passphrase_missing(void) {
    // device unlocked with a passphrase, words checked without it
    fake_crypto_setup(REFERENCE_PASSPHRASE_ROOT_KEY);
    add_speculos_mnemonic();

    TEST_ASSERT_FALSE(check_mnemonic());
}

static void test_check_mnemonic_passphrase_wrong(void) {
    const char passphrase[] = "ledger 25th word!";
    fake_crypto_setup(REFERENCE_PASSPHRASE_ROOT_KEY);
    add_speculos_mnemonic();
    TEST_ASSERT_TRUE(set_mnemonic_passphrase(passphrase, strlen(passphrase)));

    TEST_ASSERT_FALSE(check_mnemonic());
    TEST_ASSERT_EQUAL(0, get_mnemonic_passphrase_length());
}

static void test_check_mnemonic_passphrase_unexpected(void) {
    // device holding the plain seed, words checked with a passphrase
    add_speculos_mnemonic();
    TEST_ASSERT_TRUE(set_mnemonic_passphrase(REFERENCE_PASSPHRASE,
                                             strlen(REFERENCE_PASSPHRASE)));

    TEST_ASSERT_FALSE(check_mnemonic());
}

static void test_check_mnemonic_passphrase_empty(void) {
    // an empty passphrase is the plain BIP39 seed
    add_speculos_mnemonic();
    TEST_ASSERT_TRUE(set_mnemonic_passphrase("", 0));

    TEST_ASSERT_TRUE(check_mnemonic());
}

static void test_set_mnemonic_passphrase_too_long(void) {
    char passphrase[MAX_PASSPHRASE_LENGTH + 1];
    memset(passphrase, 'a', sizeof(passphrase));

    TEST_ASSERT_TRUE(
        set_mnemonic_passphrase(passphrase, MAX_PASSPHRASE_LENGTH));
    TEST_ASSERT_EQUAL(MAX_PASSPHRASE_LENGTH, get_mnemonic_passphrase_length());

    TEST_ASSERT_FALSE(set_mnemonic_passphrase(passphrase, sizeof(passphrase)));
    TEST_ASSERT_EQUAL(0, get_mnemonic_passphrase_length());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mnemonic_init);
    RUN_TEST(test_mnemonic_final_size);
    RUN_TEST(test_add_word_in_mnemonic);
    RUN_TEST(test_remove_word_from_mnemonic_none);
    RUN_TEST(test_remove_word_from_mnemonic);
    RUN_TEST(test_is_mnemonic_complete);
    RUN_TEST(test_check_mnemonic_nok);
    RUN_TEST(test_check_mnemonic_nok2);
    RUN_TEST(test_check_mnemonic_ok);
    RUN_TEST(test_check_mnemonic_not_device_seed);
    RUN_TEST(test_check_mnemonic_passphrase_ok);
    RUN_TEST(test_check_mnemonic_passphrase_missing);
    RUN_TEST(test_check_mnemonic_passphrase_wrong);
    RUN_TEST(test_check_mnemonic_passphrase_unexpected);
    RUN_TEST(test_check_mnemonic_passphrase_empty);
    RUN_TEST(test_set_mnemonic_passphrase_too_long);
    return UNITY_END();
}
