//
//  shamir.c
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#include <string.h>
#include <cx.h>

#include "shamir.h"
#include "interpolate.h"

#define memzero(...) explicit_bzero(__VA_ARGS__)

//////////////////////////////////////////////////
// hmac sha256

uint8_t *create_digest(const uint8_t *random_data,
                       uint32_t rdlen,
                       const uint8_t *shared_secret,
                       uint32_t sslen,
                       uint8_t *result) {
    uint8_t buf[32];

    cx_hmac_sha256(random_data, rdlen, shared_secret, sslen, buf, sizeof(buf));

    for (uint8_t j = 0; j < 4; ++j) {
        result[j] = buf[j];
    }

    return result;
}

static int16_t validate_parameters(uint8_t threshold, uint8_t share_count, uint8_t secret_length) {
    if (share_count > SHAMIR_MAX_SHARE_COUNT) {
        return SHAMIR_ERROR_TOO_MANY_SHARES;
    } else if (threshold < 1 || threshold > share_count) {
        return SHAMIR_ERROR_INVALID_THRESHOLD;
    } else if (secret_length > SHAMIR_MAX_SECRET_SIZE) {
        return SHAMIR_ERROR_SECRET_TOO_LONG;
    } else if (secret_length < SHAMIR_MIN_SECRET_SIZE) {
        return SHAMIR_ERROR_SECRET_TOO_SHORT;
    } else if (secret_length & 1) {
        return SHAMIR_ERROR_SECRET_NOT_EVEN_LEN;
    }
    return 0;
}

//////////////////////////////////////////////////
// shamir sharing
int16_t shamir_split_secret(uint8_t threshold,
                            uint8_t share_count,
                            const uint8_t *secret,
                            uint8_t secret_length,
                            uint8_t *result,
                            unsigned char *(*random_generator)(uint8_t *, size_t)) {
    int32_t err = validate_parameters(threshold, share_count, secret_length);
    if (err) {
        return err;
    }

    if (threshold == 1) {
        // just return share_count copies of the secret
        uint8_t *share = result;
        for (uint8_t i = 0; i < share_count; ++i, share += secret_length) {
            for (uint8_t j = 0; j < secret_length; ++j) {
                share[j] = secret[j];
            }
        }
        return share_count;
    } else {
        uint8_t digest[secret_length];
        uint8_t x[16];
        const uint8_t *y[16];
        uint8_t n = 0;
        uint8_t *share = result;

        for (uint8_t i = 0; i < threshold - 2; ++i, share += secret_length) {
            random_generator(share, secret_length);
            x[n] = i;
            y[n] = share;
            n += 1;
        }

        // generate secret_length - 4 bytes worth of random data
        random_generator(digest + 4, secret_length - 4);
        // put 4 bytes of digest at the top of the digest array
        create_digest(digest + 4, secret_length - 4, secret, secret_length, digest);
        x[n] = DIGEST_INDEX;
        y[n] = digest;
        n += 1;

        x[n] = SECRET_INDEX;
        y[n] = secret;
        n += 1;

        for (uint8_t i = threshold - 2; i < share_count; ++i, share += secret_length) {
            if (interpolate(n, x, secret_length, y, i, share) < 0) {
                return SHAMIR_ERROR_INTERPOLATION_FAILURE;
            }
        }

        memzero(digest, sizeof(digest));
        memzero(x, sizeof(x));
        memzero(y, sizeof(y));
    }
    return share_count;
}

// returns the number of bytes written to the secret array, or a negative value if there was an
// error
int16_t shamir_recover_secret(uint8_t threshold,
                              const uint8_t *x,
                              const uint8_t **shares,
                              uint8_t share_length,
                              uint8_t *secret) {
    int32_t err = validate_parameters(threshold, threshold, share_length);
    if (err) {
        return err;
    }

    uint8_t digest[share_length];
    uint8_t verify[4];
    uint8_t valid = 1;

    if (threshold == 1) {
        for (uint8_t j = 0; j < share_length; ++j) {
            secret[j] = shares[0][j];
        }
        return share_length;
    }

    if (interpolate(threshold, x, share_length, shares, DIGEST_INDEX, digest) < 0 ||
        interpolate(threshold, x, share_length, shares, SECRET_INDEX, secret) < 0) {
        memzero(secret, sizeof(digest));
        memzero(digest, sizeof(digest));
        memzero(verify, sizeof(verify));

        return SHAMIR_ERROR_INTERPOLATION_FAILURE;
    }

    create_digest(digest + 4, share_length - 4, secret, share_length, verify);

    for (uint8_t i = 0; i < 4; i++) {
        valid &= digest[i] == verify[i];
    }

    memzero(digest, sizeof(digest));
    memzero(verify, sizeof(verify));

    if (!valid) {
        return SHAMIR_ERROR_CHECKSUM_FAILURE;
    }

    return share_length;
}
