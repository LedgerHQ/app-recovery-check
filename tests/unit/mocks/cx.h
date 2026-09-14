#pragma once

#include <stdint.h>
#include <sodium.h>
#include <stdlib.h>
#include <string.h>

#include "os.h"

#define CX_LAST 0

/*
 * Host-side doubles for the Ledger cryptographic primitives used by the
 * mnemonic verification path. They must compute the exact bytes the real API
 * produces: a mock returning CX_OK without writing its output buffer leaves
 * compare_recovery_phrase() comparing uninitialized stack memory, which is
 * undefined behaviour and makes the security-sensitive path untestable.
 */

// Mirrors include/cx_errors.h from the SDK: CX_CHECK() assigns to a local
// 'error' and jumps to the 'end' label on failure.
#define CX_INTERNAL_ERROR 0xFFFFFF85
typedef uint32_t cx_err_t;

#define CX_CHECK(call) \
    do {               \
        error = call;  \
        if (error) {   \
            goto end;  \
        }              \
    } while (0)

typedef struct cx_hmac_sha512_s {
    crypto_auth_hmacsha512_state state;
} cx_hmac_sha512_t;
typedef cx_hmac_sha512_t cx_hmac_t;

size_t cx_hash_sha256(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len) {
    crypto_hash_sha256(out, in, in_len);
    return 32;
}

size_t cx_hash_sha512(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len) {
    crypto_hash_sha512(out, in, in_len);
    return 64;
}

static int mock_hmac_sha512(const uint8_t *key,
                            size_t key_len,
                            const uint8_t *in,
                            size_t in_len,
                            uint8_t mac[static 64]) {
    crypto_auth_hmacsha512_state state;

    if (crypto_auth_hmacsha512_init(&state, key, key_len) != 0) {
        return CX_INVALID_PARAMETER;
    }
    if (crypto_auth_hmacsha512_update(&state, in, in_len) != 0) {
        return CX_INVALID_PARAMETER;
    }
    if (crypto_auth_hmacsha512_final(&state, mac) != 0) {
        return CX_INVALID_PARAMETER;
    }
    return CX_OK;
}

/*
 * PBKDF2-HMAC-SHA512 (RFC 8018). The Ledger API takes the bare salt and
 * appends the big-endian block index itself, which is why callers size their
 * salt buffer with 4 extra bytes; this mock appends it the same way and never
 * reads past saltlen.
 */
int cx_pbkdf2_sha512(const uint8_t *password,
                     size_t passwordlen,
                     uint8_t *salt,
                     size_t saltlen,
                     uint32_t iterations,
                     uint8_t *out,
                     size_t outLength) {
    uint8_t block[64];
    uint8_t acc[64];
    uint8_t *salted = NULL;

    if (password == NULL || salt == NULL || out == NULL || outLength == 0 || iterations == 0) {
        return CX_INVALID_PARAMETER;
    }

    salted = (uint8_t *) malloc(saltlen + 4);
    if (salted == NULL) {
        return CX_INVALID_PARAMETER;
    }
    memcpy(salted, salt, saltlen);

    for (uint32_t counter = 1; (counter - 1) * 64 < outLength; counter++) {
        const size_t offset = (size_t) (counter - 1) * 64;
        const size_t chunk = (outLength - offset) < 64 ? (outLength - offset) : 64;

        salted[saltlen + 0] = (uint8_t) (counter >> 24);
        salted[saltlen + 1] = (uint8_t) (counter >> 16);
        salted[saltlen + 2] = (uint8_t) (counter >> 8);
        salted[saltlen + 3] = (uint8_t) counter;

        if (mock_hmac_sha512(password, passwordlen, salted, saltlen + 4, block) != CX_OK) {
            free(salted);
            return CX_INVALID_PARAMETER;
        }
        memcpy(acc, block, sizeof(acc));

        for (uint32_t round = 1; round < iterations; round++) {
            if (mock_hmac_sha512(password, passwordlen, block, sizeof(block), block) != CX_OK) {
                free(salted);
                return CX_INVALID_PARAMETER;
            }
            for (size_t i = 0; i < sizeof(acc); i++) {
                acc[i] ^= block[i];
            }
        }
        memcpy(out + offset, acc, chunk);
    }

    free(salted);
    return CX_OK;
}

int cx_hmac_sha512_init_no_throw(cx_hmac_sha512_t *ctx, const uint8_t *key, size_t size) {
    if (ctx == NULL || key == NULL) {
        return CX_INVALID_PARAMETER;
    }
    if (crypto_auth_hmacsha512_init(&ctx->state, key, size) != 0) {
        return CX_INVALID_PARAMETER;
    }
    return CX_OK;
}

int cx_hmac_no_throw(cx_hmac_t *hmac,
                     uint32_t mode,
                     const uint8_t *in,
                     size_t len,
                     uint8_t *mac,
                     size_t mac_len) {
    if (hmac == NULL || in == NULL || mac == NULL || mac_len < 64) {
        return CX_INVALID_PARAMETER;
    }
    if (crypto_auth_hmacsha512_update(&hmac->state, in, len) != 0) {
        return CX_INVALID_PARAMETER;
    }
    // the production code only ever finalizes, with CX_LAST
    if (crypto_auth_hmacsha512_final(&hmac->state, mac) != 0) {
        return CX_INVALID_PARAMETER;
    }
    return CX_OK;
}
