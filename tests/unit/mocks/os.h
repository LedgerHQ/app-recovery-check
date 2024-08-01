#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define PRINTF printf
#define CX_OK 0
#define cx_curve_t int

#define LEDGER_ASSERT(a, b) (a)

int os_secure_memcmp(const void *s1, const void *s2, size_t n);

int os_derive_bip32_no_throw(cx_curve_t curve, const unsigned int *path, unsigned int path_len,
                             unsigned char raw_privkey[static 64], unsigned char *chain_code);
