/*
* The test seeds and shares used for these tests have been taken from:
*         https://github.com/BlockchainCommons/crypto-commons/blob/master/Docs/sskr-test-vector.md#256-bit-seed
*
* The seed is generated from the following BIP39 words:
*         toe priority custom gauge jacket theme arrest bargain
*         gloom wide ill fit eagle prepare capable fish limb
*         cigar reform other priority speak rough imitate
* 
* The set of 2-of-3 shares as SSKR ByteWords are:
*         tuna acid epic hard data love wolf able acid able
*         duty surf belt task judo legs ruby cost belt pose
*         ruby logo iron vows luck bald user lazy tuna belt
*         guru buzz limp exam obey kept task cash saga pool
*         love brag roof owls news junk
* 
*         tuna acid epic hard data love wolf able acid acid
*         barn peck luau keys each duty waxy quad open bias
*         what cusp zaps math kick dark join nail legs oboe
*         also twin yank road very blue gray saga oboe city
*         gear beta quad draw knob main
* 
*         tuna acid epic hard data love wolf able acid also
*         fund able city road whiz zone claw high frog work
*         deli slot gush cats kiwi gyro numb puma join fund
*         when math inch even curl rich vows oval also unit
*         brew door atom love gyro figs
*/

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "shamir.h"
#include "interpolate.h"

unsigned char *fake_rng(uint8_t *buffer, size_t len);

const uint8_t seed[] = {0xE3, 0x95, 0x5C, 0xDA, 0x30, 0x47, 0x71, 0xC0,
                        0x03, 0x18, 0x95, 0x63, 0x7F, 0x55, 0xC3, 0xAB,
                        0xE4, 0x51, 0x53, 0xC8, 0x7A, 0xBD, 0x81, 0xC5,
                        0x1E, 0xD1, 0x4E, 0x8A, 0xAF, 0xA1, 0xAF, 0x13};


static void test_shamir_recover(void **state) {

    const uint8_t share_1[] = {0x30, 0xCC, 0x0D, 0xCF, 0x70, 0x83, 0xBD, 0x1F,
                               0x0D, 0xAF, 0xBD, 0x88, 0x69, 0xE8, 0x8C, 0x0B,
                               0xDF, 0x81, 0xD9, 0x0D, 0x53, 0x15, 0x85, 0x37, 
                               0xA1, 0x77, 0xCF, 0x17, 0xC2, 0xAE, 0x8A, 0x12};

    const uint8_t share_2[] = {0x0C, 0xAA, 0x8B, 0x78, 0x31, 0x30, 0xEE, 0xB3,
                               0xA5, 0x0F, 0xF0, 0x22, 0xFA, 0x90, 0x79, 0x24,
                               0x6D, 0x99, 0x83, 0xA2, 0x02, 0xDA, 0xF5, 0xBA,
                               0xE1, 0x10, 0x51, 0xC2, 0xA2, 0x1A, 0x4B, 0x0E};

    const uint8_t share_3[] = {0x48, 0x00, 0x1A, 0xBA, 0xF2, 0xFE, 0x1B, 0x5C,
                               0x46, 0xF4, 0x27, 0xC7, 0x54, 0x18, 0x7D, 0x55,
                               0xA0, 0xB1, 0x6D, 0x48, 0xF1, 0x90, 0x65, 0x36,
                               0x21, 0xB9, 0xE8, 0xA6, 0x02, 0xDD, 0x13, 0x2A};

    uint8_t member_indexs[] = {0x00, 0x01};
    uint8_t threshold = sizeof(member_indexs);
    uint8_t share_length = sizeof(seed);
    uint8_t secret[share_length];

    const uint8_t *shares[] = {share_1, share_2};

    int recovery = shamir_recover_secret(threshold,
                                         member_indexs,
                                         shares,
                                         share_length,
                                         secret);

    assert_int_equal(recovery, share_length);
    assert_memory_equal(secret, seed, share_length);

    shares[0] = share_3;
    member_indexs[0] = 0x02;

    recovery = shamir_recover_secret(threshold,
                                     member_indexs,
                                     shares,
                                     share_length,
                                     secret);

    assert_int_equal(recovery, share_length);
    assert_memory_equal(secret, seed, share_length);

    shares[1] = share_1;
    member_indexs[1] = 0x00;

    recovery = shamir_recover_secret(threshold,
                                     member_indexs,
                                     shares,
                                     share_length,
                                     secret);

    assert_int_equal(recovery, share_length);
    assert_memory_equal(secret, seed, share_length);

    shares[0] = share_1;
    member_indexs[0] = 0x01;

    recovery = shamir_recover_secret(threshold,
                                     member_indexs,
                                     shares,
                                     share_length,
                                     secret);

    assert_int_equal(recovery, SHAMIR_ERROR_CHECKSUM_FAILURE);
    assert_memory_not_equal(secret, seed, share_length);
}

static void test_shamir_split(void **state) {
    const uint8_t shares[] = {0xFF, 0x0F, 0xBB, 0x2A, 0x8B, 0xCC, 0x6F, 0x00,
                              0xC8, 0xE6, 0x83, 0xDF, 0xB0, 0xEB, 0x5F, 0x1C,
                              0x55, 0xEF, 0x12, 0xD9, 0x4B, 0x62, 0x97, 0x42,
                              0x42, 0xDA, 0x21, 0x11, 0xBA, 0xC6, 0x5F, 0xAA,
                              0xB4, 0x8E, 0x81, 0x9F, 0xBB, 0x8A, 0x1C, 0xC3,
                              0xCF, 0xFB, 0x10, 0xBB, 0xC7, 0xB7, 0x96, 0xBC,
                              0xBD, 0xB3, 0x4F, 0x1E, 0x21, 0xCE, 0x04, 0x94,
                              0x48, 0x1E, 0x79, 0x8C, 0x0D, 0x7E, 0xEA, 0xA2,
                              0x69, 0x16, 0xCF, 0x5B, 0xEB, 0x40, 0x89, 0x9D,
                              0xC6, 0xDC, 0xBE, 0x17, 0x5E, 0x53, 0xD6, 0x47,
                              0x9E, 0x57, 0xA8, 0x4C, 0x9F, 0x21, 0xAA, 0xF5,
                              0x56, 0x49, 0x91, 0x30, 0xCF, 0xAD, 0x2E, 0xBA};

    uint8_t threshold = 2;
    const uint8_t share_count = 3;
    const uint8_t seed_length = sizeof(seed);
    uint8_t result[seed_length * share_count];
 
    int32_t ret_val = shamir_split_secret(threshold,
                                          share_count,
                                          seed,
                                          seed_length,
                                          result,
                                          fake_rng);

    assert_int_equal(ret_val, share_count);
    assert_memory_equal(result, shares, sizeof(result));

    threshold = 4;
    ret_val = shamir_split_secret(threshold,
                                  share_count,
                                  seed,
                                  seed_length,
                                  result,
                                  fake_rng);

    assert_int_equal(ret_val, SHAMIR_ERROR_INVALID_THRESHOLD);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_shamir_recover),
        cmocka_unit_test(test_shamir_split)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
