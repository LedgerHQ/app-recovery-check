#include "os.h"

int os_secure_memcmp(const void *s1, const void *s2, size_t n)
{
    return memcmp(s1, s2, n);
}

int os_derive_bip32_no_throw(cx_curve_t curve, const unsigned int *path, unsigned int path_len,
                             unsigned char raw_privkey[static 64], unsigned char *chain_code)
{
    return 0;
}
