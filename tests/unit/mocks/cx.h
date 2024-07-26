#pragma once

#include <stdint.h>
#include <sodium.h>

#include "os.h"


#define CX_CURVE_256K1 0
#define CX_LAST 0

typedef struct cx_hmac_sha512_s {
    uint8_t not_empty;
} cx_hmac_sha512_t;
typedef struct cx_hmac_s {
    uint8_t not_empty;
} cx_hmac_t;

size_t cx_hash_sha256(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len)
{
    crypto_hash_sha256(out, in, in_len);
    return 32;
}

size_t cx_hash_sha512(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len)
{
    crypto_hash_sha512(out, in, in_len);
    return 64;
}


int cx_pbkdf2_sha512(const uint8_t *password,
                     size_t         passwordlen,
                     uint8_t       *salt,
                     size_t         saltlen,
                     uint32_t       iterations,
                     uint8_t       *out,
                     size_t         outLength) {
    return 0;
}

int cx_hmac_sha512_init_no_throw(void* ptr, const uint8_t* key, size_t size)
{
    return CX_OK;
}

int cx_hmac_no_throw(cx_hmac_t     *hmac,
                     uint32_t       mode,
                     const uint8_t *in,
                     size_t         len,
                     uint8_t       *mac,
                     size_t         mac_len)
{
    return 0;
}
