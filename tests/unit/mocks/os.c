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

int os_derive_bip32_no_throw(cx_curve_t curve, const unsigned int* path,
                             unsigned int path_len,
                             unsigned char raw_privkey[static 64],
                             unsigned char* chain_code) {
    return 0;
}
