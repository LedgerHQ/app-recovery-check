//
//  interpolate.c
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#include "os.h"
#include "ox_bn.h"
#include "cx_errors.h"

#include "interpolate.h"

// The irreducible polynomial N(x) = x^8 + x^4 + x^3 + x + 1
#define SSS_POLYNOMIAL \
    { 0x01, 0x1B }

// 2nd Montgomery constant: R2 = x^(2*t*8) mod N(x)
// t = 1 since the number of bytes of R is 1.
#define MONTGOMERY_CONSTANT_R2 \
    { 0xA1 }

// Minimal required bytes for BN storing a GF(256) value
#define GF2_8_MPI_BYTES 1

#define memzero(...) explicit_bzero(__VA_ARGS__)

#ifdef TARGET_NANOS
/**
 * @brief Performs a multiplication over GF(2^n).
 *
 * @param[out] bn_r BN index for the result.
 *
 * @param[in]  bn_a BN index of the first operand.
 *
 * @param[in]  bn_b BN index of the second operand.
 *
 * @param[in]  bn_n BN index of the modulus.
 *                  The modulus must be an irreducible polynomial over GF(2)
 *                  of degree n.
 *
 * @param[in]  bn_h BN index of the second montgomery constant.
 *
 * @return          Error code:
 *                  - CX_OK on success
 *                  - CX_NOT_LOCKED
 *                  - CX_INVALID_PARAMETER
 *                  - CX_MEMORY_FULL
 */
cx_err_t cx_bn_gf2_n_mul(cx_bn_t bn_r,
                         const cx_bn_t bn_a,
                         const cx_bn_t bn_b,
                         const cx_bn_t bn_n,
                         const cx_bn_t bn_h __attribute__((unused))) {
    cx_err_t error = CX_OK;
    cx_bn_t bn_x, bn_y, bn_temp;
    int cmp_x, cmp_y;
    uint32_t degree = 0;
    size_t nbytes;
    bool bit_set = 0;

    // Preliminaries
    CX_CHECK(cx_bn_nbytes(bn_n, &nbytes));
    CX_CHECK(cx_bn_alloc(&bn_x, nbytes));
    CX_CHECK(cx_bn_alloc(&bn_y, nbytes));
    CX_CHECK(cx_bn_alloc(&bn_temp, nbytes));
    CX_CHECK(cx_bn_copy(bn_x, bn_a));
    CX_CHECK(cx_bn_copy(bn_y, bn_b));

    // Calculate the degree of the modulus polynomial
    CX_CHECK(cx_bn_copy(bn_temp, bn_n));
    do {
        CX_CHECK(cx_bn_cmp_u32(bn_temp, (uint32_t) 0, &cmp_x));
        CX_CHECK(cx_bn_shr(bn_temp, 1));
    } while (cmp_x != 0 && ++degree);

    // After loop degree is offset by 1
    degree--;
    if (degree < 1) {
        error = CX_INVALID_PARAMETER;
        goto end;
    }

    // Ensure both operands are in field
    CX_CHECK(cx_bn_shr(bn_x, degree));
    CX_CHECK(cx_bn_shr(bn_y, degree));
    // Maybe change cx_bn_cmp_u32 to cx_bn_cnt_bits
    CX_CHECK(cx_bn_cmp_u32(bn_x, (uint32_t) 0, &cmp_x));
    CX_CHECK(cx_bn_cmp_u32(bn_y, (uint32_t) 0, &cmp_y));

    if (cmp_x != 0 || cmp_y != 0) {
        error = CX_INVALID_PARAMETER;
        goto end;
    }

    // Check if both operands are non-zero
    CX_CHECK(cx_bn_copy(bn_x, bn_a));
    CX_CHECK(cx_bn_copy(bn_y, bn_b));
    // Maybe cx_bn_cmp_u32 change to cx_bn_cnt_bits
    CX_CHECK(cx_bn_cmp_u32(bn_x, (uint32_t) 0, &cmp_x));
    CX_CHECK(cx_bn_cmp_u32(bn_y, (uint32_t) 0, &cmp_y));

    CX_CHECK(cx_bn_set_u32(bn_r, (uint32_t) 0));

    // Main loop for multiplication
    while (cmp_x != 0 && cmp_y != 0) {
        CX_CHECK(cx_bn_tst_bit(bn_y, 0, &bit_set));
        if (bit_set) {
            CX_CHECK(cx_bn_copy(bn_temp, bn_r));
            CX_CHECK(cx_bn_xor(bn_r, bn_x, bn_temp));
        }

        CX_CHECK(cx_bn_shl(bn_x, 1));
        CX_CHECK(cx_bn_tst_bit(bn_x, degree, &bit_set));

        if (bit_set) {
            CX_CHECK(cx_bn_copy(bn_temp, bn_x));
            CX_CHECK(cx_bn_xor(bn_x, bn_n, bn_temp));
        }

        CX_CHECK(cx_bn_shr(bn_y, 1));

        // Maybe change cx_bn_cmp_u32 to cx_bn_cnt_bits
        CX_CHECK(cx_bn_cmp_u32(bn_x, (uint32_t) 0, &cmp_x));
        CX_CHECK(cx_bn_cmp_u32(bn_y, (uint32_t) 0, &cmp_y));
    }

    // Clean up
    CX_CHECK(cx_bn_destroy(&bn_x));
    CX_CHECK(cx_bn_destroy(&bn_y));
    CX_CHECK(cx_bn_destroy(&bn_temp));

end:
    return error;
}
#endif

/*
 * Invert `bn_a` in GF(2^8) and write the result to `bn_r`
 */
cx_err_t bn_gf2_8_inv(cx_bn_t bn_r, const cx_bn_t bn_a, const cx_bn_t bn_n, const cx_bn_t bn_h) {
    cx_err_t error = CX_OK;  // By default, until some error occurs
    cx_bn_t bn_x, bn_y, bn_z;

    CX_CHECK(cx_bn_alloc(&bn_x, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_alloc(&bn_y, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_alloc(&bn_z, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_copy(bn_x, bn_a));

    CX_CHECK(cx_bn_gf2_n_mul(bn_y, bn_x, bn_x, bn_n, bn_h));  // bn_y = bn_x^2
    CX_CHECK(cx_bn_gf2_n_mul(bn_y, bn_y, bn_y, bn_n, bn_h));  // bn_y = bn_x^4
    CX_CHECK(cx_bn_gf2_n_mul(bn_r, bn_y, bn_y, bn_n, bn_h));  // bn_r = bn_x^8
    CX_CHECK(cx_bn_gf2_n_mul(bn_z, bn_r, bn_x, bn_n, bn_h));  // bn_z = bn_x^9
    CX_CHECK(cx_bn_gf2_n_mul(bn_r, bn_r, bn_r, bn_n, bn_h));  // bn_r = bn_x^16
    CX_CHECK(cx_bn_gf2_n_mul(bn_r, bn_r, bn_z, bn_n, bn_h));  // bn_r = bn_x^25
    CX_CHECK(cx_bn_gf2_n_mul(bn_r, bn_r, bn_r, bn_n, bn_h));  // bn_r = bn_x^50
    CX_CHECK(cx_bn_gf2_n_mul(bn_z, bn_r, bn_r, bn_n, bn_h));  // bn_z = bn_x^100
    CX_CHECK(cx_bn_gf2_n_mul(bn_z, bn_z, bn_z, bn_n, bn_h));  // bn_z = bn_x^200
    CX_CHECK(cx_bn_gf2_n_mul(bn_r, bn_r, bn_z, bn_n, bn_h));  // bn_r = bn_x^250
    CX_CHECK(cx_bn_gf2_n_mul(bn_r, bn_r, bn_y, bn_n, bn_h));  // bn_r = bn_x^254

    CX_CHECK(cx_bn_destroy(&bn_x));
    CX_CHECK(cx_bn_destroy(&bn_y));
    CX_CHECK(cx_bn_destroy(&bn_z));

end:
    return error;
}

/**
 * safely interpolate the polynomial going through
 * the points (x0 [y0_0 y0_1 y0_2 ... y0_31]) , (x1 [y1_0 ...]), ...
 *
 * where
 *   xi points to [x0 x1 ... xn-1 ]
 *   y contains an array of pointers to 32-bit arrays of y values
 *   y contains [y0 y1 y2 ... yn-1]
 *   and each of the yi arrays contain [yi_0 yi_i ... yi_31].
 *
 * returns: on success, CX_OK
 *          on failure, a negative error code
 *
 * inputs: n: number of points to interpolate
 *         xi: x coordinates for points (array of length n)
 *         yl: length of y coordinate arrays
 *         yij: array of n pointers to arrays of length yl
 *         x: coordinate to interpolate at
 *         result: space for yl bytes of interpolate data
 */
cx_err_t interpolate(uint8_t n,            // number of points to interpolate
                     const uint8_t *xi,    // x coordinates for points (array of length n)
                     uint8_t yl,           // length of y coordinate array
                     const uint8_t **yij,  // n arrays of yl bytes representing y values
                     uint8_t x,            // x coordinate to interpolate
                     uint8_t *result       // space for yl bytes of results
) {
    const uint8_t N[2] = SSS_POLYNOMIAL;
    const uint8_t R2[1] = MONTGOMERY_CONSTANT_R2;

    cx_err_t error = CX_OK;  // By default, until some error occurs
    cx_bn_t bn_x, bn_xc_i, bn_xc_j;
    cx_bn_t bn_numerator, bn_denominator;
    cx_bn_t bn_lagrange, bn_y, bn_result, bn_temp, bn_n, bn_r2;
    uint32_t result_u32;

    CX_CHECK(cx_bn_lock(GF2_8_MPI_BYTES, 0));
    CX_CHECK(cx_bn_alloc(&bn_x, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_alloc(&bn_xc_i, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_alloc(&bn_xc_j, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_alloc(&bn_numerator, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_alloc(&bn_denominator, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_alloc(&bn_lagrange, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_alloc(&bn_y, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_alloc(&bn_result, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_alloc(&bn_temp, GF2_8_MPI_BYTES));
    CX_CHECK(cx_bn_alloc_init(&bn_n, GF2_8_MPI_BYTES, N, sizeof(N)));
    CX_CHECK(cx_bn_alloc_init(&bn_r2, GF2_8_MPI_BYTES, R2, sizeof(R2)));

    CX_CHECK(cx_bn_set_u32(bn_x, (uint32_t) x));
    memzero(result, yl);

    for (uint8_t i = 0; i < n; i++) {
        CX_CHECK(cx_bn_set_u32(bn_xc_i, (uint32_t) xi[i]));
        CX_CHECK(cx_bn_set_u32(bn_lagrange, (uint32_t) 1));

        // calculate the lagrange basis coefficient for the lagrange polynomial
        // defined by the x coordinates xi at the value x.
        //
        // After loop runs, bn_lagrange should hold data satisfying
        // the following:
        //                ---     (x-xi[j])
        // bn_lagrange =  | |   -------------
        //              j != i  (xi[i]-xi[j])
        for (uint8_t j = 0; j < n; j++) {
            if (j != i) {
                CX_CHECK(cx_bn_set_u32(bn_xc_j, (uint32_t) xi[j]));

                // Calculate the numerator (x - xc[j])
                CX_CHECK(cx_bn_xor(bn_numerator, bn_x, bn_xc_j));

                // Calculate the denominator (xc[i] - xc[j])
                CX_CHECK(cx_bn_xor(bn_denominator, bn_xc_i, bn_xc_j));

                // Calculate the inverse of the denominator
                CX_CHECK(bn_gf2_8_inv(bn_denominator, bn_denominator, bn_n, bn_r2));

                // Calculate the lagrange basis coefficient
                CX_CHECK(cx_bn_gf2_n_mul(bn_lagrange, bn_numerator, bn_lagrange, bn_n, bn_r2));
                CX_CHECK(cx_bn_gf2_n_mul(bn_lagrange, bn_denominator, bn_lagrange, bn_n, bn_r2));
            }
        }

        for (uint8_t j = 0; j < yl; j++) {
            CX_CHECK(cx_bn_set_u32(bn_y, (uint32_t) yij[i][j]));
            CX_CHECK(cx_bn_set_u32(bn_result, (uint32_t) result[j]));

            CX_CHECK(cx_bn_gf2_n_mul(bn_y, bn_lagrange, bn_y, bn_n, bn_r2));
            CX_CHECK(cx_bn_copy(bn_temp, bn_result));
            CX_CHECK(cx_bn_xor(bn_result, bn_temp, bn_y));
            CX_CHECK(cx_bn_get_u32(bn_result, &result_u32));
            result[j] = (uint8_t) result_u32;
            result_u32 = 0;
        }
    }

    // clean up stack
    CX_CHECK(cx_bn_destroy(&bn_x));
    CX_CHECK(cx_bn_destroy(&bn_xc_i));
    CX_CHECK(cx_bn_destroy(&bn_xc_j));
    CX_CHECK(cx_bn_destroy(&bn_numerator));
    CX_CHECK(cx_bn_destroy(&bn_denominator));
    CX_CHECK(cx_bn_destroy(&bn_lagrange));
    CX_CHECK(cx_bn_destroy(&bn_y));
    CX_CHECK(cx_bn_destroy(&bn_result));
    CX_CHECK(cx_bn_destroy(&bn_temp));
    CX_CHECK(cx_bn_destroy(&bn_n));
    CX_CHECK(cx_bn_destroy(&bn_r2));

end:
    if (cx_bn_is_locked()) {
        cx_bn_unlock();
    }

    return error;
}
