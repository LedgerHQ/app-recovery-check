#include <stdint.h>
#include <sodium.h>

size_t cx_hash_sha256(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len __attribute__((unused)))
{
    crypto_hash_sha256(out, in, in_len);
    return 32;
}

size_t cx_hash_sha512(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len __attribute__((unused)))
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
