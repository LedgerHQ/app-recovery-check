//
//  sskr.c
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "sskr.h"
#include "shard.h"
#include "sss.h"

#define memzero(...) explicit_bzero(__VA_ARGS__)

static int16_t sskr_check_secret_length(uint8_t len) {
    if (len < SSKR_MIN_STRENGTH_BYTES) {
        return SSKR_ERROR_SECRET_TOO_SHORT;
    }
    if (len > SSKR_MAX_STRENGTH_BYTES) {
        return SSKR_ERROR_SECRET_TOO_LONG;
    }
    if (len & 1) {
        return SSKR_ERROR_SECRET_LENGTH_NOT_EVEN;
    }
    return 0;
}

static int16_t sskr_serialize_shard(const sskr_shard_t *shard,
                                    uint8_t *destination,
                                    uint16_t destination_len) {
    if (destination_len < SSKR_METADATA_LENGTH_BYTES + shard->value_len) {
        return SSKR_ERROR_INSUFFICIENT_SPACE;
    }

    // pack the id, group and member data into 5 bytes:
    // 76543210        76543210        76543210
    //         76543210        76543210
    // ----------------====----====----====----
    // identifier: 16
    //                 group-threshold: 4
    //                     group-count: 4
    //                         group-index: 4
    //                             member-threshold: 4
    //                                 reserved (MUST be zero): 4
    //                                     member-index: 4

    uint16_t id = shard->identifier;
    uint8_t gt = (shard->group_threshold - 1) & 0xf;
    uint8_t gc = (shard->group_count - 1) & 0xf;
    uint8_t gi = shard->group_index & 0xf;
    uint8_t mt = (shard->member_threshold - 1) & 0xf;
    uint8_t mi = shard->member_index & 0xf;

    uint8_t id1 = id >> 8;
    uint8_t id2 = id & 0xff;

    destination[0] = id1;
    destination[1] = id2;
    destination[2] = (gt << 4) | gc;
    destination[3] = (gi << 4) | mt;
    destination[4] = mi;

    memcpy(destination + SSKR_METADATA_LENGTH_BYTES, shard->value, shard->value_len);

    return shard->value_len + SSKR_METADATA_LENGTH_BYTES;
}

static int16_t sskr_deserialize_shard(const uint8_t *source,
                                      uint16_t source_len,
                                      sskr_shard_t *shard) {
    if (source_len < SSKR_MIN_SERIALIZED_LENGTH_BYTES) {
        return SSKR_ERROR_NOT_ENOUGH_SERIALIZED_BYTES;
    }

    uint8_t group_threshold = (source[2] >> 4) + 1;
    uint8_t group_count = (source[2] & 0xf) + 1;

    if (group_threshold > group_count) {
        return SSKR_ERROR_INVALID_GROUP_THRESHOLD;
    }

    shard->identifier = ((uint16_t) source[0]) << 8 | source[1];
    shard->group_threshold = group_threshold;
    shard->group_count = group_count;
    shard->group_index = source[3] >> 4;
    shard->member_threshold = (source[3] & 0xf) + 1;
    uint8_t reserved = source[4] >> 4;
    if (reserved != 0) {
        return SSKR_ERROR_INVALID_RESERVED_BITS;
    }
    shard->member_index = source[4] & 0xf;
    shard->value_len = source_len - SSKR_METADATA_LENGTH_BYTES;
    memcpy(shard->value, source + SSKR_METADATA_LENGTH_BYTES, shard->value_len);

    int16_t err = sskr_check_secret_length(shard->value_len);
    if (err) {
        return err;
    }
    return shard->value_len;
}

int16_t sskr_count_shards(uint8_t group_threshold,
                          const sskr_group_descriptor_t *groups,
                          uint8_t groups_len) {
    uint8_t shard_count = 0;

    if (groups_len < 1) {
        return SSKR_ERROR_INVALID_GROUP_LENGTH;
    }

    if (group_threshold > groups_len) {
        return SSKR_ERROR_INVALID_GROUP_THRESHOLD;
    }

    for (uint8_t i = 0; i < groups_len; ++i) {
        if (groups[i].count < 1) {
            return SSKR_ERROR_INVALID_GROUP_COUNT;
        }
        shard_count += groups[i].count;
        if (groups[i].threshold > groups[i].count) {
            return SSKR_ERROR_INVALID_MEMBER_THRESHOLD;
        }
        if (groups[i].threshold == 1 && groups[i].count > 1) {
            return SSKR_ERROR_INVALID_SINGLETON_MEMBER;
        }
    }

    return shard_count;
}

//////////////////////////////////////////////////
// generate shards
//
static int16_t sskr_generate_shards(uint8_t group_threshold,
                                    const sskr_group_descriptor_t *groups,
                                    uint8_t groups_len,
                                    const uint8_t *master_secret,
                                    uint16_t master_secret_len,
                                    sskr_shard_t *shards,
                                    uint16_t shards_size,
                                    unsigned char *(*random_generator)(uint8_t *, size_t)) {
    int16_t err = sskr_check_secret_length(master_secret_len);
    if (err) {
        return err;
    }

    // Figure out how many shards we are dealing with
    int16_t total_shards = sskr_count_shards(group_threshold, groups, groups_len);
    if (total_shards < 0) {
        return total_shards;
    }

    // assign a random identifier
    uint16_t identifier = 0;
    random_generator((uint8_t *) (&identifier), 2);

    if (shards_size < total_shards) {
        return SSKR_ERROR_INSUFFICIENT_SPACE;
    }

    if (group_threshold > groups_len) {
        return SSKR_ERROR_INVALID_GROUP_THRESHOLD;
    }

    uint8_t group_shares[SSS_MAX_SECRET_SIZE * SSKR_MAX_GROUP_COUNT];

    sss_split_secret(group_threshold,
                     groups_len,
                     master_secret,
                     master_secret_len,
                     group_shares,
                     random_generator);

    uint8_t *group_share = group_shares;

    uint16_t shards_count = 0;
    sskr_shard_t *shard;

    for (uint8_t i = 0; i < groups_len; ++i, group_share += master_secret_len) {
        uint8_t member_shares[SSS_MAX_SECRET_SIZE * SSS_MAX_SHARE_COUNT];
        sss_split_secret(groups[i].threshold,
                         groups[i].count,
                         group_share,
                         master_secret_len,
                         member_shares,
                         random_generator);

        uint8_t *value = member_shares;
        for (uint8_t j = 0; j < groups[i].count; ++j, value += master_secret_len) {
            shard = &shards[shards_count];

            shard->identifier = identifier;
            shard->group_threshold = group_threshold;
            shard->group_count = groups_len;
            shard->value_len = master_secret_len;
            shard->group_index = i;
            shard->member_threshold = groups[i].threshold;
            shard->member_index = j;
            memzero(shard->value, 32);
            memcpy(shard->value, value, master_secret_len);

            shards_count++;
        }

        // clean up
        memzero(member_shares, sizeof(member_shares));
    }

    // clean up stack
    memzero(group_shares, sizeof(group_shares));

    // return the number of shards generated
    return shards_count;
}

//////////////////////////////////////////////////
// generate mnemonics
//
int16_t sskr_generate(uint8_t group_threshold,
                      const sskr_group_descriptor_t *groups,
                      uint8_t groups_len,
                      const uint8_t *master_secret,
                      uint16_t master_secret_len,
                      uint8_t *shard_len,
                      uint8_t *output,
                      uint16_t buffer_size,
                      unsigned char *(*random_generator)(uint8_t *, size_t)) {
    int16_t err = sskr_check_secret_length(master_secret_len);
    if (err) {
        return err;
    }

    // Figure out how many shards we are dealing with
    int16_t total_shards = sskr_count_shards(group_threshold, groups, groups_len);
    if (total_shards < 0) {
        return total_shards;
    }

    // figure out how much space we need to store all of the mnemonics
    // and make sure that we were provided with sufficient resources
    uint16_t shard_length = SSKR_METADATA_LENGTH_BYTES + master_secret_len;
    if (buffer_size < shard_length * total_shards) {
        return SSKR_ERROR_INSUFFICIENT_SPACE;
    }

    int16_t error = 0;

    // allocate space for shard representations
    sskr_shard_t shards[SSS_MAX_SHARE_COUNT * SSKR_MAX_GROUP_COUNT];

    // generate shards
    total_shards = sskr_generate_shards(group_threshold,
                                        groups,
                                        groups_len,
                                        master_secret,
                                        master_secret_len,
                                        shards,
                                        (uint16_t) total_shards,
                                        random_generator);

    if (total_shards < 0) {
        error = total_shards;
    }

    uint8_t *cur_output = output;
    uint16_t remaining_buffer = buffer_size;
    uint16_t byte_count = 0;

    for (uint16_t i = 0; !error && i < (uint16_t) total_shards; ++i) {
        int16_t bytes = sskr_serialize_shard(&shards[i], cur_output, remaining_buffer);
        if (bytes < 0) {
            error = bytes;
            break;
        }
        byte_count = bytes;
        remaining_buffer -= byte_count;
        cur_output += byte_count;
    }

    memzero(shards, sizeof(shards));
    if (error) {
        memzero(output, buffer_size);
        return 0;
    }

    *shard_len = byte_count;
    return total_shards;
}

typedef struct sskr_group_struct {
    uint8_t group_index;
    uint8_t member_threshold;
    uint8_t count;
    uint8_t member_index[SSS_MAX_SHARE_COUNT];
    const uint8_t *value[SSS_MAX_SHARE_COUNT];
} sskr_group;

/**
 * This version of combine shards potentially modifies the shard structures
 * in place, so it is for internal use only, however it provides the implementation
 * for both combine_shards and sskr_combine.
 */
static int16_t sskr_combine_shards_internal(
    sskr_shard_t *shards,  // array of shard structures
    uint8_t shards_count,  // number of shards in array
    uint8_t *buffer,       // working space, and place to return secret
    uint16_t buffer_len    // total amount of working space
) {
    int16_t error = 0;
    uint16_t identifier = 0;
    uint8_t group_threshold = 0;
    uint8_t group_count = 0;

    if (shards_count == 0) {
        return SSKR_ERROR_EMPTY_SHARD_SET;
    }

    uint8_t next_group = 0;
    sskr_group groups[SSKR_MAX_GROUP_COUNT];
    uint8_t secret_len = 0;

    for (uint8_t i = 0; i < shards_count; ++i) {
        sskr_shard_t *shard = &shards[i];

        if (i == 0) {
            // on the first one, establish expected values for common metadata
            identifier = shard->identifier;
            group_count = shard->group_count;
            group_threshold = shard->group_threshold;
            secret_len = shard->value_len;
        } else {
            // on subsequent shards, check that common metadata matches
            if (shard->identifier != identifier || shard->group_threshold != group_threshold ||
                shard->group_count != group_count || shard->value_len != secret_len) {
                return SSKR_ERROR_INVALID_SHARD_SET;
            }
        }

        // sort shards into member groups
        bool group_found = false;
        for (uint8_t j = 0; j < next_group; ++j) {
            if (shard->group_index == groups[j].group_index) {
                group_found = true;
                if (shard->member_threshold != groups[j].member_threshold) {
                    return SSKR_ERROR_INVALID_MEMBER_THRESHOLD;
                }
                for (uint8_t k = 0; k < groups[j].count; ++k) {
                    if (shard->member_index == groups[j].member_index[k]) {
                        return SSKR_ERROR_DUPLICATE_MEMBER_INDEX;
                    }
                }
                groups[j].member_index[groups[j].count] = shard->member_index;
                groups[j].value[groups[j].count] = shard->value;
                groups[j].count++;
            }
        }

        if (!group_found) {
            sskr_group *g = &groups[next_group];
            g->group_index = shard->group_index;
            g->member_threshold = shard->member_threshold;
            g->count = 1;
            g->member_index[0] = shard->member_index;
            g->value[0] = shard->value;
            next_group++;
        }
    }

    if (buffer_len < secret_len) {
        error = SSKR_ERROR_INSUFFICIENT_SPACE;
    } else if (next_group < group_threshold) {
        error = SSKR_ERROR_NOT_ENOUGH_GROUPS;
    }

    // here, all of the shards are unpacked into member groups. Now we go through each
    // group and recover the group secret, and then use the result to recover the
    // master secret
    uint8_t gx[SSKR_MAX_GROUP_COUNT];
    const uint8_t *gy[SSKR_MAX_GROUP_COUNT];

    // allocate enough space for the group shards and the encrypted master secret
    uint8_t group_shares[SSKR_MAX_STRENGTH_BYTES * (SSKR_MAX_GROUP_COUNT + 1)];

    uint8_t *group_share = group_shares;

    for (uint8_t i = 0; !error && i < (uint8_t) next_group; ++i) {
        sskr_group *g = &groups[i];

        gx[i] = g->group_index;
        if (g->count < g->member_threshold) {
            error = SSKR_ERROR_NOT_ENOUGH_MEMBER_SHARDS;
            break;
        }

        int16_t recovery = sss_recover_secret(g->member_threshold,
                                              g->member_index,
                                              g->value,
                                              secret_len,
                                              group_share);

        if (recovery < 0) {
            error = recovery;
            break;
        }
        gy[i] = group_share;

        group_share += recovery;
    }

    int16_t recovery = 0;
    if (!error) {
        recovery = sss_recover_secret(group_threshold, gx, gy, secret_len, group_share);
    }

    if (recovery < 0) {
        error = recovery;
    }

    // copy the result to the beginning of the buffer supplied
    if (!error) {
        memcpy(buffer, group_share, secret_len);
    }

    // clean up stack
    memzero(group_shares, sizeof(group_shares));
    memzero(gx, sizeof(gx));
    memzero(gy, sizeof(gy));
    memzero(groups, sizeof(groups));

    if (error) {
        return error;
    }

    return secret_len;
}

/////////////////////////////////////////////////
// sskr_combine

int16_t sskr_combine(const uint8_t **input_shards,  // array of pointers to 10-bit words
                     uint8_t shard_len,             // number of bytes in each serialized shard
                     uint8_t shards_count,          // total number of shards
                     uint8_t *buffer,               // working space, and place to return secret
                     uint16_t buffer_len            // total amount of working space
) {
    int16_t result = 0;

    if (shards_count == 0) {
        return SSKR_ERROR_EMPTY_SHARD_SET;
    }

    sskr_shard_t shards[SSS_MAX_SHARE_COUNT * SSKR_MAX_GROUP_COUNT];

    for (uint16_t i = 0; !result && i < shards_count; ++i) {
        shards[i].value_len = 32;

        int16_t bytes = sskr_deserialize_shard(input_shards[i], shard_len, &shards[i]);

        if (bytes < 0) {
            result = bytes;
        }
    }

    if (!result) {
        result = sskr_combine_shards_internal(shards, shards_count, buffer, buffer_len);
    }

    memzero(shards, sizeof(shards));

    return result;
}
