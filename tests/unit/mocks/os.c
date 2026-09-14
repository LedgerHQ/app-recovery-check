#include "os.h"

// Mirrors the no-early-exit semantics of the Ledger OS primitive: the whole
// buffer is always read, whatever the position of the first difference. Tests
// exercising compare_recovery_phrase() must not be linked against an
// early-exit comparator such as memcmp().
int os_secure_memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* a = (const unsigned char*)s1;
    const unsigned char* b = (const unsigned char*)s2;
    unsigned char diff = 0;

    for (size_t i = 0; i < n; i++) {
        diff |= (unsigned char)(a[i] ^ b[i]);
    }
    return diff;
}

// BIP32 master key of the default Speculos mnemonic ("glory promote mansion
// ... raise skin"), the seed the tests assume the device holds. Computed
// independently of the application code, so that a regression in the BIP39
// derivation shows up as a mismatch rather than cancelling itself out:
//   seed = PBKDF2-HMAC-SHA512(SHA512(mnemonic), "mnemonic", 2048, 64)
//   root = HMAC-SHA512("Bitcoin seed", seed)
// The inner SHA512 is the Ledger-specific pre-hash applied to mnemonics
// longer than 128 bytes, see bolos_ux_mnemonic_to_seed_hash_length128().
const unsigned char REFERENCE_ROOT_KEY[BIP32_ROOT_KEY_SIZE] = {
    0x34, 0xac, 0x5d, 0x78, 0x4e, 0xbb, 0x4d, 0xf4, 0x72, 0x7b, 0xcd,
    0xdf, 0x6a, 0x67, 0x43, 0xf5, 0xd5, 0xd4, 0x6d, 0x83, 0xdd, 0x74,
    0xaa, 0x82, 0x58, 0x66, 0x39, 0x0c, 0x69, 0x4f, 0x29, 0x38, 0xeb,
    0x47, 0x3a, 0x0f, 0xa0, 0xaf, 0x50, 0x31, 0xf1, 0x4d, 0xb9, 0xfe,
    0x7c, 0x37, 0xbb, 0x84, 0x16, 0xa4, 0xff, 0x01, 0xbb, 0x69, 0xda,
    0xe9, 0x96, 0x6d, 0xc8, 0x3b, 0x5e, 0x5b, 0xf9, 0x21};

// Only the master key of the reference seed is modelled, which is all
// compare_recovery_phrase() asks for (empty path). Anything else fails rather
// than returning CX_OK with an untouched output buffer.
int os_derive_bip32_no_throw(cx_curve_t curve, const unsigned int* path,
                             unsigned int path_len,
                             unsigned char raw_privkey[static 64],
                             unsigned char* chain_code) {
    if (curve != CX_CURVE_256K1 || path_len != 0 || raw_privkey == NULL ||
        chain_code == NULL) {
        return CX_INVALID_PARAMETER;
    }
    memcpy(raw_privkey, REFERENCE_ROOT_KEY, 32);
    memcpy(chain_code, REFERENCE_ROOT_KEY + 32, 32);
    return CX_OK;
}
