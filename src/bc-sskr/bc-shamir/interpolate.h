//
//  interpolate.h
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#ifndef INTERPOLATE_H
#define INTERPOLATE_H

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
                     const uint8_t* xi,    // x coordinates for points (array of length n)
                     uint8_t yl,           // length of y coordinate array
                     const uint8_t** yij,  // n arrays of yl bytes representing y values
                     uint8_t x,            // x coordinate to interpolate
                     uint8_t* result       // space for yl bytes of results
);

#endif /* INTERPOLATE_H */
