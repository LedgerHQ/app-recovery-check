//
//  sskr.h
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#ifndef SSKR_H
#define SSKR_H

#include <stdlib.h>
#include <stdint.h>

#include "sskr-constants.h"
#include "group.h"

int16_t sskr_count_shards(uint8_t group_threshold,
                          const sskr_group_descriptor_t *groups,
                          uint8_t groups_len);

/**
 * generate a set of shards that can be used to reconstruct a secret
 * using the given group policy.
 *
 * returns: the number of shards generated if successful,
 *          or a negative number indicating an error code when unsuccessful
 *
 * inputs: group_threshold: the number of groups that need to be satisfied in order
 *                          to reconstruct the secret
 *         groups: an array of group descriptors
 *         groups_length: the length of the groups array
 *         master_secret: pointer to the secret to split up
 *         master_secret_length: length of the master secret in bytes.
 *                               must be >= 16, <= 32, and even.
 *         shard_len: pointer to an integer that will be filled with the number of
 *                          bytes in each shard
 *         output: array of bytes to store the resulting shards.
 *                    the ith shard will be represented by
 *                     output[i*shard_len]..output[(i+1)*shard_len -1]
 *         buffer_size: maximum number of bytes to write to the output array
 */
int16_t sskr_generate(uint8_t group_threshold,
                      const sskr_group_descriptor_t *groups,
                      uint8_t groups_length,
                      const uint8_t *master_secret,
                      uint16_t master_secret_length,
                      uint8_t *shard_len,
                      uint8_t *output,
                      uint16_t buffer_size,
                      unsigned char *(*random_generator)(uint8_t *, size_t));

/**
 * combine a set of serialized shards to reconstruct a secret
 *
 * returns: the length of the reconstructed secret if successful
 *          or a negative number indicating an error code when unsuccessful
 *
 * inputs: input_shards: an array of pointers to serialized shards
 *         shard_len: number of bytes in each serialized shard
 *         shards_count: total number of shards
 *         buffer: location to store the result
 *         buffer_length: maximum space available in buffer
 */
int16_t sskr_combine(const uint8_t **input_shards,  // an array of pointers to serialized shards
                     uint8_t shard_len,             // number of bytes in each serialized shard
                     uint8_t shards_count,          // total number of shards
                     uint8_t *buffer,               // working space, and place to return secret
                     uint16_t buffer_length         // total amount of working space
);

#endif /* SSKR_H */
