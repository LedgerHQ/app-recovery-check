#include "fake_crypto.h"

#include <sodium.h>
#include <stdlib.h>
#include <string.h>

#include "Mocklcx_hmac.h"
#include "Mocklcx_pbkdf2.h"
#include "Mocklcx_sha256.h"
#include "Mockos_seed.h"
#include "Mockos_utils.h"
#include "cx.h"

/*
 * BIP32 master key of the default Speculos mnemonic ("glory promote mansion
 * ... raise skin"), computed independently of the application code, so that a
 * regression in the BIP39 derivation shows up as a mismatch:
 *   seed = PBKDF2-HMAC-SHA512(mnemonic, "mnemonic", 2048, 64)
 *   root = HMAC-SHA512("Bitcoin seed", seed)
 */
const uint8_t REFERENCE_ROOT_KEY[BIP32_ROOT_KEY_SIZE] = {
    0x34, 0xac, 0x5d, 0x78, 0x4e, 0xbb, 0x4d, 0xf4, 0x72, 0x7b, 0xcd,
    0xdf, 0x6a, 0x67, 0x43, 0xf5, 0xd5, 0xd4, 0x6d, 0x83, 0xdd, 0x74,
    0xaa, 0x82, 0x58, 0x66, 0x39, 0x0c, 0x69, 0x4f, 0x29, 0x38, 0xeb,
    0x47, 0x3a, 0x0f, 0xa0, 0xaf, 0x50, 0x31, 0xf1, 0x4d, 0xb9, 0xfe,
    0x7c, 0x37, 0xbb, 0x84, 0x16, 0xa4, 0xff, 0x01, 0xbb, 0x69, 0xda,
    0xe9, 0x96, 0x6d, 0xc8, 0x3b, 0x5e, 0x5b, 0xf9, 0x21};

// Same mnemonic with REFERENCE_PASSPHRASE:
//   seed = PBKDF2-HMAC-SHA512(mnemonic, "mnemonic" || passphrase, 2048, 64)
const char REFERENCE_PASSPHRASE[] = "Ledger 25th word!";
const uint8_t REFERENCE_PASSPHRASE_ROOT_KEY[BIP32_ROOT_KEY_SIZE] = {
    0x2f, 0xdc, 0x0a, 0x3f, 0x0a, 0x77, 0x3a, 0x16, 0xac, 0x2d, 0x35,
    0x6b, 0x70, 0xd8, 0x8f, 0xba, 0x43, 0x7c, 0x4e, 0x54, 0x3b, 0x11,
    0xc9, 0x2d, 0xc2, 0xe3, 0x72, 0x9f, 0x5b, 0x85, 0x38, 0x5d, 0x9d,
    0x27, 0x5b, 0x11, 0x39, 0xe0, 0xa2, 0x40, 0x1b, 0xad, 0x49, 0x78,
    0x21, 0x58, 0xbb, 0x47, 0x5f, 0x43, 0x3c, 0xad, 0x69, 0xdd, 0x32,
    0x5c, 0x1d, 0x8a, 0xed, 0xfe, 0x12, 0xc8, 0x3f, 0x1e};

// Master key returned by the derivation, i.e. the device active seed
static const uint8_t* device_root_key;

// Single HMAC context of the app, keyed by cx_hmac_sha512_init_no_throw()
static const cx_hmac_sha512_t* hmac_ctx;
static crypto_auth_hmacsha512_state hmac_state;

static size_t fake_hash_sha256(const uint8_t* in, size_t in_len, uint8_t* out,
                               size_t out_len, int cmock_num_calls) {
    (void)cmock_num_calls;
    if (out_len < crypto_hash_sha256_BYTES) {
        return 0;
    }
    crypto_hash_sha256(out, in, in_len);
    return crypto_hash_sha256_BYTES;
}

// Not mocked: lcx_sha512.h is not self-contained
size_t cx_hash_sha512(const uint8_t* in, size_t in_len, uint8_t* out,
                      size_t out_len) {
    if (out_len < crypto_hash_sha512_BYTES) {
        return 0;
    }
    crypto_hash_sha512(out, in, in_len);
    return crypto_hash_sha512_BYTES;
}

static cx_err_t fake_hmac_sha512_init(cx_hmac_sha512_t* hmac,
                                      const uint8_t* key, size_t key_len,
                                      int cmock_num_calls) {
    (void)cmock_num_calls;
    if (hmac == NULL || key == NULL ||
        crypto_auth_hmacsha512_init(&hmac_state, key, key_len) != 0) {
        return CX_INVALID_PARAMETER;
    }
    hmac_ctx = hmac;
    return CX_OK;
}

// Only the one-shot use of the app is modelled: a single update, with CX_LAST
static cx_err_t fake_hmac(cx_hmac_t* hmac, uint32_t mode, const uint8_t* in,
                          size_t len, uint8_t* mac, size_t mac_len,
                          int cmock_num_calls) {
    uint8_t out[crypto_auth_hmacsha512_BYTES];
    (void)cmock_num_calls;

    if ((const void*)hmac != (const void*)hmac_ctx || mode != CX_LAST ||
        in == NULL || mac == NULL || mac_len < sizeof(out)) {
        return CX_INVALID_PARAMETER;
    }
    hmac_ctx = NULL;
    if (crypto_auth_hmacsha512_update(&hmac_state, in, len) != 0 ||
        crypto_auth_hmacsha512_final(&hmac_state, out) != 0) {
        return CX_INVALID_PARAMETER;
    }
    // in and mac may overlap
    memcpy(mac, out, sizeof(out));
    sodium_memzero(out, sizeof(out));
    return CX_OK;
}

static int hmac_sha512(const uint8_t* key, size_t key_len, const uint8_t* in,
                       size_t in_len,
                       uint8_t mac[crypto_auth_hmacsha512_BYTES]) {
    crypto_auth_hmacsha512_state state;

    if (crypto_auth_hmacsha512_init(&state, key, key_len) != 0 ||
        crypto_auth_hmacsha512_update(&state, in, in_len) != 0 ||
        crypto_auth_hmacsha512_final(&state, mac) != 0) {
        return -1;
    }
    return 0;
}

// PBKDF2-HMAC-SHA512 (RFC 8018), the block index appended to the salt
static cx_err_t fake_pbkdf2(cx_md_t md_type, const uint8_t* password,
                            size_t passwordlen, uint8_t* salt, size_t saltlen,
                            uint32_t iterations, uint8_t* out, size_t outLength,
                            int cmock_num_calls) {
    uint8_t block[crypto_auth_hmacsha512_BYTES];
    uint8_t acc[crypto_auth_hmacsha512_BYTES];
    uint8_t* salted = NULL;
    cx_err_t error = CX_OK;
    (void)cmock_num_calls;

    if (md_type != CX_SHA512 || password == NULL || salt == NULL ||
        out == NULL || outLength == 0 || iterations == 0) {
        return CX_INVALID_PARAMETER;
    }
    salted = malloc(saltlen + 4);
    if (salted == NULL) {
        return CX_INVALID_PARAMETER;
    }
    memcpy(salted, salt, saltlen);

    for (uint32_t counter = 1;
         (size_t)(counter - 1) * sizeof(block) < outLength; counter++) {
        const size_t offset = (size_t)(counter - 1) * sizeof(block);
        const size_t chunk = (outLength - offset) < sizeof(block)
                                 ? (outLength - offset)
                                 : sizeof(block);

        salted[saltlen + 0] = (uint8_t)(counter >> 24);
        salted[saltlen + 1] = (uint8_t)(counter >> 16);
        salted[saltlen + 2] = (uint8_t)(counter >> 8);
        salted[saltlen + 3] = (uint8_t)counter;

        if (hmac_sha512(password, passwordlen, salted, saltlen + 4, block) !=
            0) {
            error = CX_INVALID_PARAMETER;
            break;
        }
        memcpy(acc, block, sizeof(acc));
        for (uint32_t round = 1; round < iterations; round++) {
            if (hmac_sha512(password, passwordlen, block, sizeof(block),
                            block) != 0) {
                error = CX_INVALID_PARAMETER;
                break;
            }
            for (size_t i = 0; i < sizeof(acc); i++) {
                acc[i] ^= block[i];
            }
        }
        if (error != CX_OK) {
            break;
        }
        memcpy(out + offset, acc, chunk);
    }

    free(salted);
    return error;
}

// Only the device master key is modelled (empty path), anything else throws
static void fake_derive_node(unsigned int mode, cx_curve_t curve,
                             const unsigned int* path, unsigned int pathLength,
                             unsigned char* privateKey, unsigned char* chain,
                             unsigned char* seed_key,
                             unsigned int seed_key_length,
                             int cmock_num_calls) {
    (void)seed_key;
    (void)cmock_num_calls;
    if (mode != HDW_NORMAL || curve != CX_CURVE_256K1 || path == NULL ||
        pathLength != 0 || seed_key_length != 0 || privateKey == NULL ||
        chain == NULL || device_root_key == NULL) {
        THROW(CX_INVALID_PARAMETER);
    }
    memcpy(privateKey, device_root_key, 32);
    memcpy(chain, device_root_key + 32, 32);
}

// Same semantics as the device: the whole buffer is always read
static char fake_secure_memcmp(const void* src1, const void* src2,
                               size_t length, int cmock_num_calls) {
    const uint8_t* a = src1;
    const uint8_t* b = src2;
    uint8_t diff = 0;
    (void)cmock_num_calls;

    for (size_t i = 0; i < length; i++) {
        diff |= a[i] ^ b[i];
    }
    return (char)diff;
}

void fake_crypto_setup(const uint8_t* root_key) {
    device_root_key = root_key;
    hmac_ctx = NULL;
    cx_hash_sha256_StubWithCallback(fake_hash_sha256);
    cx_hmac_sha512_init_no_throw_StubWithCallback(fake_hmac_sha512_init);
    cx_hmac_no_throw_StubWithCallback(fake_hmac);
    cx_pbkdf2_no_throw_StubWithCallback(fake_pbkdf2);
    os_perso_derive_node_with_seed_key_StubWithCallback(fake_derive_node);
    os_secure_memcmp_StubWithCallback(fake_secure_memcmp);
}
