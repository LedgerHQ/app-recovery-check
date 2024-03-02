//
//  sss.h
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#ifndef SSS_H
#define SSS_H

#include <stdint.h>
#include "sss-constants.h"

#define SSS_SECRET_INDEX 255
#define SSS_DIGEST_INDEX 254

//////////////////////////////////////////////////
// Shamir Secret Sharing (based on SLIP-39)

/**
 * uses SLIP-39's strategy for shamir sharing to split a secret up into
 * share_count shares such that threshold of them must be presented
 * to recover the secret.
 *
 * returns: the number of shares created, or a negative value if there was an error
 *
 * inputs: threshold: number of shares required to recover secret. Must be 1 <= threshold <=
 * share_count. share_count: number of shares to generate secret: array of bytes representing the
 * secret secret_length: length of the secret array. must be >= 16, <= 32 and even. result: place to
 * store the resulting shares. Must be able to hold share_count * secret_length bytes
 */
int16_t sss_split_secret(uint8_t threshold,
                         uint8_t share_count,
                         const uint8_t *secret,
                         uint8_t secret_length,
                         uint8_t *result,
                         unsigned char *(*random_generator)(uint8_t *, size_t));

/**
 * recover a secret from shares
 *
 * returns: the number of bytes written to the secret array, or a negative value if there was an
 * error
 *
 * inputs: threshold: number of shares required and provided to this function
 *         x: array of x values (length: threshold)
 *         shares: array (length: threshold) of pointers to y value arrays
 *         share_length: number of bytes in each y value array
 *         secret: array for writing results (must be at least share_length long)
 */
int16_t sss_recover_secret(uint8_t threshold,
                           const uint8_t *x,
                           const uint8_t **shares,
                           uint8_t share_length,
                           uint8_t *secret);

#endif /* SSS_H */
