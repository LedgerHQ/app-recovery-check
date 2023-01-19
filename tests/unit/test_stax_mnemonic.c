#include <stdarg.h>
/* #include <stddef.h> */
#include <setjmp.h>
#include <stdint.h>
/* #include <stdbool.h> */
#include <string.h>
#include <cmocka.h>

#include "stax/ux_stax.h"


static int setup(void **state __attribute__((unused))) {
    // resets the whole buffer with initial values
    reset_mnemonic();
    return 0;
}

static void test_mnemonic_init(void **state __attribute__((unused))) {
    assert_int_equal(get_mnemonic_final_size(), 0);
    assert_int_equal(get_current_word_number(), 0);
    assert_string_equal(get_mnemonic(), "");
    assert_false(is_mnemonic_complete());
    assert_false(check_mnemonic());
}

static void test_mnemonic_final_size(void **state __attribute__((unused))) {
    const size_t test = 9;
    set_mnemonic_final_size(test);
    assert_int_equal(get_mnemonic_final_size(), test);
}

static void test_add_word_in_mnemonic(void **state __attribute__((unused))) {
    const char* word = "hello";
    assert_int_equal(get_current_word_number(), 0);
    assert_string_equal(get_mnemonic(), "");

    assert_int_equal(add_word_in_mnemonic(word, 5), 1);
    assert_int_equal(get_current_word_number(), 1);
    assert_string_equal(get_mnemonic(), word);
}

static void test_remove_word_from_mnemonic_none(void **state __attribute__((unused))) {
    assert_false(remove_word_from_mnemonic());
}

static void test_remove_word_from_mnemonic(void **state __attribute__((unused))) {
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

static void test_is_mnemonic_complete(void **state __attribute__((unused))) {
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

static void test_check_mnemonic_nok(void **state __attribute__((unused))) {
    assert_false(check_mnemonic());

    set_mnemonic_final_size(1);
    assert_false(check_mnemonic());

    assert_int_equal(add_word_in_mnemonic("word", 4), 1);
    assert_string_equal(get_mnemonic(), "word");
    assert_false(check_mnemonic());
}

static void test_check_mnemonic_ok(void **state __attribute__((unused))) {
    const char* const mnemonic[] = {"list", "of", "random", "words", "which", "actually", "are", "the", "mnemonic"};
    int i = 0;
    size_t mnemonic_size = (sizeof(mnemonic) / sizeof(char*));
    set_mnemonic_final_size(mnemonic_size);

    for (i = 0; i < mnemonic_size; i++) {
        assert_int_equal(add_word_in_mnemonic(mnemonic[i], strlen(mnemonic[i])), i + 1);
    }

    assert_true(check_mnemonic());
}


int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_mnemonic_init, setup, NULL),
        cmocka_unit_test_setup_teardown(test_mnemonic_final_size, setup, NULL),
        cmocka_unit_test_setup_teardown(test_add_word_in_mnemonic, setup, NULL),
        cmocka_unit_test_setup_teardown(test_remove_word_from_mnemonic_none, setup, NULL),
        cmocka_unit_test_setup_teardown(test_remove_word_from_mnemonic, setup, NULL),
        cmocka_unit_test_setup_teardown(test_is_mnemonic_complete, setup, NULL),
        cmocka_unit_test_setup_teardown(test_check_mnemonic_nok, setup, NULL),
        cmocka_unit_test_setup_teardown(test_check_mnemonic_ok, setup, NULL),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
