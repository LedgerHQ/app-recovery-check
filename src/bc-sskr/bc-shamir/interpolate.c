//
//  interpolate.c
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#include <string.h>

#include "interpolate.h"
#include "hazmat.h"
#include "shamir-constants.h"

#define memzero(...) explicit_bzero(__VA_ARGS__)

/*
 * calculate the lagrange basis coefficients for the lagrange polynomial
 * defined byt the x coordinates xc at the value x.
 *
 * inputs: values: pointer to an array to write the values
 *         n: number of points - length of the xc array, 0 < n <= 32
 *         xc: array of x components to use as interpolating points
 *         x: x coordinate to evaluate lagrange polynomials at
 *
 * After the function runs, the values array should hold data satisfying
 * the following:
 *                ---     (x-xc[j])
 *   values[i] =  | |   -------------
 *              j != i  (xc[i]-xc[j])
 */
static void hazmat_lagrange_basis(uint8_t *values, uint8_t n, const uint8_t *xc, uint8_t x) {
    // call the contents of xc [ x0 x1 x2 ... xn-1 ]
    uint8_t xx[32 + 16];
    uint8_t i;

    uint32_t x_slice[8], lxi[SHAMIR_MAX_SHARE_COUNT][8];

    uint32_t numerator[8], denominator[8], temp[8];

    memzero(xx, sizeof(xx));
    for (i = 0; i < n; ++i) {
        xx[i] = xc[i];
    }

    // xx now contains bitsliced [ x0 x1 x2 ... xn-1 0 0 0 ... ]
    for (i = 0; i < n; ++i) {
        // lxi = bitsliced [ xi xi+1 xi+2 ... xi-1 0 0 0 ]
        bitslice(&(lxi[i][0]), &(xx[i]));
        xx[i + n] = xx[i];
    }

    bitslice_setall(x_slice, x);
    bitslice_setall(numerator, 1);
    bitslice_setall(denominator, 1);

    for (i = 1; i < n; ++i) {
        memcpy(temp, x_slice, sizeof(temp));
        gf256_add(temp, &(lxi[i][0]));
        // temp = [ x-xi+i x-xi+2 x-xi+3 ... x-xi x x x]
        gf256_mul(numerator, numerator, temp);

        memcpy(temp, &(lxi[0][0]), sizeof(temp));
        gf256_add(temp, &(lxi[i][0]));
        // temp = [x0-xi+1 x1-xi+1 x2-xi+2 ... xn-x0 0 0 0]
        gf256_mul(denominator, denominator, temp);
    }

    // At this stage the numerator contains
    // [ num0 num1 num2 ... numn 0 0 0]
    //
    // where numi = prod(j, j!=i, x-xj )
    //
    // and the denomintor contains
    // [ d0 d1 d2 ... dn 0 0 0]
    //
    // where di = prod(j, j!=i, xi-xj)

    gf256_inv(temp, denominator);

    // gf256_inv uses exponentiaton to calculate inverse, so the zeros end up
    // remaining zeros.

    // tmp = [ 1/d0 1/d1 1/d2 ... 1/dn 0 0 0]

    gf256_mul(numerator, numerator, temp);

    // numerator now contains [ l_n_0(x) l_n_1(x) ... l_n_n-1(x) 0 0 0]
    // use the xx array to unpack it

    unbitslice(xx, numerator);

    // copy results to output array
    for (i = 0; i < n; ++i) {
        values[i] = xx[i];
    }
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
 * returns: on success, the number of bytes written to result
 *          on failure, a negative error code
 *
 * inputs: n: number of points to interpolate
 *         xi: x coordinates for points (array of length n)
 *         yl: length of y coordinate arrays
 *         yij: array of n pointers to arrays of length yl
 *         x: coordinate to interpolate at
 *         result: space for yl bytes of interpolate data
 */
int16_t interpolate(uint8_t n,            // number of points to interpolate
                    const uint8_t *xi,    // x coordinates for points (array of length n)
                    uint32_t yl,          // length of y coordinate array
                    const uint8_t **yij,  // n arrays of yl bytes representing y values
                    uint8_t x,            // x coordinate to interpolate
                    uint8_t *result       // space for yl bytes of results
) {
    // The hazmat gf256 implementation needs the y-coordinate data
    // to be in 32-byte blocks
    uint8_t *y[SHAMIR_MAX_SHARE_COUNT];
    uint8_t yv[SHAMIR_MAX_SECRET_SIZE * SHAMIR_MAX_SHARE_COUNT];
    uint8_t values[SHAMIR_MAX_SECRET_SIZE];

    memzero(yv, SHAMIR_MAX_SECRET_SIZE * n);
    for (uint8_t i = 0; i < n; i++) {
        y[i] = &yv[SHAMIR_MAX_SECRET_SIZE * i];
        memcpy(y[i], yij[i], yl);
    }

    uint8_t lagrange[SHAMIR_MAX_SHARE_COUNT];
    uint32_t y_slice[8], result_slice[8], temp[8];

    hazmat_lagrange_basis(lagrange, n, xi, x);

    bitslice_setall(result_slice, 0);

    for (uint8_t i = 0; i < n; ++i) {
        bitslice(y_slice, y[i]);
        bitslice_setall(temp, lagrange[i]);
        gf256_mul(temp, temp, y_slice);
        gf256_add(result_slice, temp);
    }

    unbitslice(values, result_slice);
    // the calling code is only expecting yl bytes back,
    memcpy(result, values, yl);

    // clean up stack
    memzero(lagrange, sizeof(lagrange));
    memzero(y_slice, sizeof(y_slice));
    memzero(result_slice, sizeof(result_slice));
    memzero(temp, sizeof(temp));
    memzero(y, sizeof(y));
    memzero(yv, sizeof(yv));
    memzero(values, sizeof(values));

    return yl;
}
