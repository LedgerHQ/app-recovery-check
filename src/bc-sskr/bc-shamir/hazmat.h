/*
 * Low level API for Daan Sprenkels' Shamir secret sharing library
 * Copyright (c) 2017 Daan Sprenkels <hello@dsprenkels.com>
 *
 * Licensed under the "MIT License"
 *
 * Usage of this API is hazardous and is only reserved for beings with a
 * good understanding of the Shamir secret sharing scheme and who know how
 * crypto code is implemented. If you are unsure about this, use the
 * intermediate level API. You have been warned!
 */

#ifndef HAZMAT_H
#define HAZMAT_H

#include <inttypes.h>

/**
 * bitslice - Transpose bits of 32 bytes to form 8 32-bit words.
 *
 * This function transposes the bits of an input 32-byte array x into
 * 8 32-bit words, stored in the output array r.
 *
 * @param r [out] 8-element array of 32-bit words, where the transposed bits will be stored.
 * @param x [in] 32-element byte array to be transposed.
 */
void bitslice(uint32_t r[8], const uint8_t x[32]);

/**
 * unbitslice - Reverse the bitslicing operation performed by bitslice().
 *
 * This function reverses the bitslicing operation, converting 8 32-bit words
 * back into a 32-byte array. It is essentially the inverse operation of the bitslice() function.
 *
 * @param r [out] 32-element byte array, where the unbitsliced data will be stored.
 * @param x [in] 8-element array of 32-bit words to be unbitsliced.
 */
void unbitslice(uint8_t r[32], const uint32_t x[8]);

/**
 * bitslice_setall - Set all bits in an 8-element 32-bit words array based on the input byte.
 *
 * This function sets the bits in the output array 'r' based on the input byte 'x'.
 * For each bit 'i' in 'x', if it is set, the corresponding 32-bit word in 'r' will have all bits
 * set; otherwise, the corresponding word will have all bits unset.
 *
 * @param r [out] 8-element array of 32-bit words, where the resulting data will be stored.
 * @param x [in] Input byte to determine the bit values in the output array.
 */
void bitslice_setall(uint32_t r[8], const uint8_t x);

/**
 * gf256_add - Add (XOR) two 8-element arrays of 32-bit words element-wise.
 *
 * This function performs an element-wise addition (XOR) of the input arrays 'r' and 'x'
 * and stores the result back in the 'r' array. The operation is performed in GF(2^8).
 *
 * @param r [in, out] 8-element array of 32-bit words, where one of the input arrays and the result
 * will be stored.
 * @param x [in] 8-element array of 32-bit words, the other input array.
 */
void gf256_add(uint32_t r[8], const uint32_t x[8]);

/**
 * gf256_mul - Multiply two bitsliced polynomials in GF(2^8) reduced by x^8 + x^4 + x^3 + x + 1.
 *
 * This function multiplies two bitsliced polynomials 'a' and 'b' in GF(2^8) and stores the result
 * in the output array 'r'. The multiplication is performed using Russian Peasant multiplication
 * and the polynomials are reduced by x^8 + x^4 + x^3 + x + 1.
 *
 * Note: 'r' and 'a' may overlap, but overlapping of 'r' and 'b' will produce an incorrect result!
 * If you need to square a polynomial, use `gf256_square` instead.
 *
 * @param r [out] 8-element array of 32-bit words, where the resulting data will be stored.
 * @param a [in] 8-element array of 32-bit words, one of the input polynomials.
 * @param b [in] 8-element array of 32-bit words, the other input polynomial.
 */
void gf256_mul(uint32_t r[8], const uint32_t a[8], const uint32_t b[8]);

/*
 * Square `x` in GF(2^8) and write the result to `r`. `r` and `x` may overlap.
 */
void gf256_square(uint32_t r[8], const uint32_t x[8]);

/*
 * Invert `x` in GF(2^8) and write the result to `r`
 */
void gf256_inv(uint32_t r[8], uint32_t x[8]);

#endif /* HAZMAT_H */
