// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>
#include <string.h>
// clang-format on

#include "nbgl/mnemonic.h"
#include "os.h"

// Default Speculos mnemonic
static const char* const SPECULOS_MNEMONIC[] = {
    "glory", "promote",  "mansion", "idle",    "axis",  "finger",
    "extra", "february", "uncover", "one",     "trip",  "resource",
    "lawn",  "turtle",   "enact",   "monster", "seven", "myth",
    "punch", "hobby",    "comfort", "wild",    "raise", "skin"};

static int setup(void** state __attribute__((unused))) {
    // resets the whole buffer with initial values
    reset_mnemonic();
    mock_set_device_root_key(REFERENCE_ROOT_KEY);
    return 0;
}

static void add_speculos_mnemonic(void) {
    const size_t mnemonic_size =
        sizeof(SPECULOS_MNEMONIC) / sizeof(SPECULOS_MNEMONIC[0]);
    set_mnemonic_final_size(mnemonic_size);
    for (size_t i = 0; i < mnemonic_size; i++) {
        assert_int_equal(add_word_in_mnemonic(SPECULOS_MNEMONIC[i],
                                              strlen(SPECULOS_MNEMONIC[i])),
                         i + 1);
    }
}

static void test_mnemonic_init(void** state __attribute__((unused))) {
    assert_int_equal(get_mnemonic_final_size(), 0);
    assert_int_equal(get_current_word_number(), 0);
    assert_string_equal(get_mnemonic(), "");
    assert_false(is_mnemonic_complete());
    assert_false(check_mnemonic());
}

static void test_mnemonic_final_size(void** state __attribute__((unused))) {
    const size_t test = 9;
    set_mnemonic_final_size(test);
    assert_int_equal(get_mnemonic_final_size(), test);
}

static void test_add_word_in_mnemonic(void** state __attribute__((unused))) {
    const char* word = "hello";
    assert_int_equal(get_current_word_number(), 0);
    assert_string_equal(get_mnemonic(), "");

    assert_int_equal(add_word_in_mnemonic(word, 5), 1);
    assert_int_equal(get_current_word_number(), 1);
    assert_string_equal(get_mnemonic(), word);
}

static void test_remove_word_from_mnemonic_none(void** state
                                                __attribute__((unused))) {
    assert_false(remove_word_from_mnemonic());
}

static void test_remove_word_from_mnemonic(void** state
                                           __attribute__((unused))) {
    const char* word = "list";
    assert_int_equal(get_current_word_number(), 0);
    assert_string_equal(get_mnemonic(), "");

    add_word_in_mnemonic(word, 4);
    assert_int_equal(get_current_word_number(), 1);
    assert_string_equal(get_mnemonic(), word);

    assert_true(remove_word_from_mnemonic());
    assert_int_equal(get_current_word_number(), 0);
    assert_string_equal(get_mnemonic(), "");
}

static void test_is_mnemonic_complete(void** state __attribute__((unused))) {
    set_mnemonic_final_size(2);
    assert_false(is_mnemonic_complete());

    const char* word = "list";
    assert_int_equal(add_word_in_mnemonic(word, 4), 1);
    assert_false(is_mnemonic_complete());

    assert_int_equal(add_word_in_mnemonic(word, 4), 2);
    assert_true(is_mnemonic_complete());

    set_mnemonic_final_size(3);
    assert_false(is_mnemonic_complete());

    set_mnemonic_final_size(1);
    assert_true(is_mnemonic_complete());

    set_mnemonic_final_size(0);
    assert_false(is_mnemonic_complete());
}

static void test_check_mnemonic_nok(void** state __attribute__((unused))) {
    assert_false(check_mnemonic());

    set_mnemonic_final_size(1);
    assert_false(check_mnemonic());

    assert_int_equal(add_word_in_mnemonic("word", 4), 1);
    assert_string_equal(get_mnemonic(), "word");
    assert_false(check_mnemonic());
}

static void test_check_mnemonic_nok2(void** state __attribute__((unused))) {
    // only 12 words of the mnemonic -> fail
    const char* const mnemonic[] = {
        "glory", "promote",  "mansion", "idle", "axis", "finger",
        "extra", "february", "uncover", "one",  "trip", "resource"};
    int i = 0;
    size_t mnemonic_size = (sizeof(mnemonic) / sizeof(char*));
    set_mnemonic_final_size(mnemonic_size);

    for (i = 0; i < mnemonic_size; i++) {
        assert_int_equal(add_word_in_mnemonic(mnemonic[i], strlen(mnemonic[i])),
                         i + 1);
    }

    assert_false(check_mnemonic());
}

static void test_check_mnemonic_ok(void** state __attribute__((unused))) {
    // Default Speculos mnemonic
    const char* const mnemonic[] = {
        "glory", "promote",  "mansion", "idle",    "axis",  "finger",
        "extra", "february", "uncover", "one",     "trip",  "resource",
        "lawn",  "turtle",   "enact",   "monster", "seven", "myth",
        "punch", "hobby",    "comfort", "wild",    "raise", "skin"};
    int i = 0;
    size_t mnemonic_size = (sizeof(mnemonic) / sizeof(char*));
    set_mnemonic_final_size(mnemonic_size);

    for (i = 0; i < mnemonic_size; i++) {
        assert_int_equal(add_word_in_mnemonic(mnemonic[i], strlen(mnemonic[i])),
                         i + 1);
    }

    // The mocks model a device holding this very mnemonic, so the comparison
    // against the device root key must succeed.
    assert_true(check_mnemonic());
    // a successful check clears the buffer as well
    assert_int_equal(get_current_word_number(), 0);
    assert_string_equal(get_mnemonic(), "");
}

static void test_check_mnemonic_not_device_seed(void** state
                                                __attribute__((unused))) {
    // A valid BIP39 mnemonic, but not the one the device holds: the checksum
    // passes, so the failure can only come from the root key comparison.
    const char* const mnemonic[] = {
        "abandon", "abandon", "abandon", "abandon", "abandon", "abandon",
        "abandon", "abandon", "abandon", "abandon", "abandon", "abandon",
        "abandon", "abandon", "abandon", "abandon", "abandon", "abandon",
        "abandon", "abandon", "abandon", "abandon", "abandon", "art"};
    size_t mnemonic_size = (sizeof(mnemonic) / sizeof(char*));
    set_mnemonic_final_size(mnemonic_size);

    for (size_t i = 0; i < mnemonic_size; i++) {
        assert_int_equal(add_word_in_mnemonic(mnemonic[i], strlen(mnemonic[i])),
                         i + 1);
    }

    assert_false(check_mnemonic());
    assert_int_equal(get_current_word_number(), 0);
    assert_string_equal(get_mnemonic(), "");
}

static void test_check_mnemonic_passphrase_ok(void** state
                                              __attribute__((unused))) {
    mock_set_device_root_key(REFERENCE_PASSPHRASE_ROOT_KEY);
    add_speculos_mnemonic();
    assert_true(set_mnemonic_passphrase(REFERENCE_PASSPHRASE,
                                        strlen(REFERENCE_PASSPHRASE)));

    assert_true(check_mnemonic());
    // the passphrase is wiped along with the mnemonic
    assert_int_equal(get_mnemonic_passphrase_length(), 0);
    assert_string_equal(get_mnemonic(), "");
}

static void test_check_mnemonic_passphrase_missing(void** state
                                                   __attribute__((unused))) {
    // device unlocked with a passphrase, words checked without it
    mock_set_device_root_key(REFERENCE_PASSPHRASE_ROOT_KEY);
    add_speculos_mnemonic();

    assert_false(check_mnemonic());
}

static void test_check_mnemonic_passphrase_wrong(void** state
                                                 __attribute__((unused))) {
    const char passphrase[] = "ledger 25th word!";
    mock_set_device_root_key(REFERENCE_PASSPHRASE_ROOT_KEY);
    add_speculos_mnemonic();
    assert_true(set_mnemonic_passphrase(passphrase, strlen(passphrase)));

    assert_false(check_mnemonic());
    assert_int_equal(get_mnemonic_passphrase_length(), 0);
}

static void test_check_mnemonic_passphrase_unexpected(void** state
                                                      __attribute__((unused))) {
    // device holding the plain seed, words checked with a passphrase
    add_speculos_mnemonic();
    assert_true(set_mnemonic_passphrase(REFERENCE_PASSPHRASE,
                                        strlen(REFERENCE_PASSPHRASE)));

    assert_false(check_mnemonic());
}

static void test_check_mnemonic_passphrase_empty(void** state
                                                 __attribute__((unused))) {
    // an empty passphrase is the plain BIP39 seed
    add_speculos_mnemonic();
    assert_true(set_mnemonic_passphrase("", 0));

    assert_true(check_mnemonic());
}

static void test_set_mnemonic_passphrase_too_long(void** state
                                                  __attribute__((unused))) {
    char passphrase[MAX_PASSPHRASE_LENGTH + 1];
    memset(passphrase, 'a', sizeof(passphrase));

    assert_true(set_mnemonic_passphrase(passphrase, MAX_PASSPHRASE_LENGTH));
    assert_int_equal(get_mnemonic_passphrase_length(), MAX_PASSPHRASE_LENGTH);

    assert_false(set_mnemonic_passphrase(passphrase, sizeof(passphrase)));
    assert_int_equal(get_mnemonic_passphrase_length(), 0);
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_mnemonic_init, setup, NULL),
        cmocka_unit_test_setup_teardown(test_mnemonic_final_size, setup, NULL),
        cmocka_unit_test_setup_teardown(test_add_word_in_mnemonic, setup, NULL),
        cmocka_unit_test_setup_teardown(test_remove_word_from_mnemonic_none,
                                        setup, NULL),
        cmocka_unit_test_setup_teardown(test_remove_word_from_mnemonic, setup,
                                        NULL),
        cmocka_unit_test_setup_teardown(test_is_mnemonic_complete, setup, NULL),
        cmocka_unit_test_setup_teardown(test_check_mnemonic_nok, setup, NULL),
        cmocka_unit_test_setup_teardown(test_check_mnemonic_nok2, setup, NULL),
        cmocka_unit_test_setup_teardown(test_check_mnemonic_ok, setup, NULL),
        cmocka_unit_test_setup_teardown(test_check_mnemonic_not_device_seed,
                                        setup, NULL),
        cmocka_unit_test_setup_teardown(test_check_mnemonic_passphrase_ok,
                                        setup, NULL),
        cmocka_unit_test_setup_teardown(test_check_mnemonic_passphrase_missing,
                                        setup, NULL),
        cmocka_unit_test_setup_teardown(test_check_mnemonic_passphrase_wrong,
                                        setup, NULL),
        cmocka_unit_test_setup_teardown(
            test_check_mnemonic_passphrase_unexpected, setup, NULL),
        cmocka_unit_test_setup_teardown(test_check_mnemonic_passphrase_empty,
                                        setup, NULL),
        cmocka_unit_test_setup_teardown(test_set_mnemonic_passphrase_too_long,
                                        setup, NULL),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
