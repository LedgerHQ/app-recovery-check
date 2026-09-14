#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define PRINTF printf
#define CX_OK  0
// any non-CX_OK value: mocks must fail loudly rather than report a success
// they did not produce
#define CX_INVALID_PARAMETER 0x6B00
#define cx_curve_t           int
#define CX_CURVE_256K1       0

// Size of a BIP32 master key: 32 bytes of private key followed by 32 bytes of
// chain code.
#define BIP32_ROOT_KEY_SIZE 64

// BIP32 master key of the reference device seed modelled by the mocks, i.e.
// the default Speculos mnemonic. See tests/unit/mocks/os.c.
extern const unsigned char REFERENCE_ROOT_KEY[BIP32_ROOT_KEY_SIZE];

#define LEDGER_ASSERT(a, b) ((void) (a))

int os_secure_memcmp(const void *s1, const void *s2, size_t n);

int os_derive_bip32_no_throw(cx_curve_t curve,
                             const unsigned int *path,
                             unsigned int path_len,
                             unsigned char raw_privkey[static 64],
                             unsigned char *chain_code);
