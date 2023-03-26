/* @BANNER@ */

#include <string.h>
#include <os.h>
#include <cx.h>

#include "onboarding_seed_rom_variables.h"
#include "common_bip39.h"
#include "bc-sskr/bc-sskr.h"

uint32_t htonl(x) uint32_t x;
{
#if BYTE_ORDER == BIG_ENDIAN
    return x;
#elif BYTE_ORDER == LITTLE_ENDIAN
    return os_swap_u32(x);
#else
#error "What kind of system is this?"
#endif
}

uint32_t crc32(const uint8_t *data, size_t len) {
    uint32_t crc = ~0;
    const uint8_t *end = data + len;

    while (data < end) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++) {
            uint32_t mask = ~((crc & 1) - 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}

// Returns the CRC-32 checksum of the input buffer in network byte order (big endian).
uint32_t crc32_nbo(const uint8_t *bytes, size_t len) {
    return htonl(crc32(bytes, len));
}

unsigned int bolos_ux_sskr_size_get(unsigned int bip39_onboarding_kind,
                                    unsigned int groups_threshold,
                                    unsigned int *group_descriptor,
                                    unsigned int groups_len,
                                    unsigned int *share_len) {
    sskr_group_descriptor groups[groups_len];
    for (uint8_t i = 0; i < groups_len; i++) {
        groups[i].threshold = *(group_descriptor + i * sizeof(*(group_descriptor) / groups_len));
        groups[i].count = *(group_descriptor + 1 + i * sizeof(*(group_descriptor) / groups_len));
    }

    unsigned int share_count_expected = sskr_count_shards(groups_threshold, groups, groups_len);
    *share_len = bip39_onboarding_kind * 4 / 3 + METADATA_LENGTH_BYTES;

    return share_count_expected;
}

unsigned int bolos_ux_sskr_generate(unsigned int groups_threshold,
                                    unsigned int *group_descriptor,
                                    unsigned int groups_len,
                                    unsigned char *seed,
                                    unsigned int seed_len,
                                    unsigned int *share_len,
                                    unsigned char *share_buffer,
                                    unsigned int share_buffer_len,
                                    unsigned int share_len_expected,
                                    unsigned int share_count_expected) {
    sskr_group_descriptor groups[groups_len];
    for (uint8_t i = 0; i < groups_len; i++) {
        groups[i].threshold = *(group_descriptor + i * 2);
        groups[i].count = *(group_descriptor + 1 + i * 2);
    }

    if (!(16 <= seed_len && seed_len <= 32) || (seed_len % 2 != 0)) {
        return 0;
    }

    PRINTF("SSKR generate input:\n %.*H\n", seed_len, seed);
    // convert seed to SSKR shares
    int share_count = sskr_generate(groups_threshold,
                                    groups,
                                    groups_len,
                                    seed,
                                    seed_len,
                                    share_len,
                                    share_buffer,
                                    share_buffer_len,
                                    NULL,
                                    (void *) cx_rng);

    if ((share_count < 0) || ((unsigned int) share_count != share_count_expected) ||
        (*share_len != share_len_expected)) {
        memset(&share_buffer, 0, share_buffer_len);
        return 0;
    }
    PRINTF("SSKR generate output:\n %.*H\n", share_buffer_len, share_buffer);
    PRINTF("SSKR share count:\n %d\n", share_count);
    PRINTF("SSKR share length :\n %d\n", *share_len);

    return share_count;
}

unsigned int bolos_ux_sskr_mnemonic_encode(unsigned char *input,
                                           unsigned int input_len,
                                           unsigned char *output,
                                           unsigned int output_len) {
    unsigned int position = 0;
    unsigned int offset = 0;

    for (uint8_t i = 0; i < input_len; i++) {
        offset = SSKR_MNEMONIC_LENGTH * input[i];
        if ((position + SSKR_MNEMONIC_LENGTH <= output_len) &&
            (offset <= SSKR_WORDLIST_LENGTH - SSKR_MNEMONIC_LENGTH)) {
            memcpy(output + position, SSKR_WORDLIST + offset, SSKR_MNEMONIC_LENGTH);
        } else {
            memset(output, 0, output_len);
            return 0;
        }
        position += SSKR_MNEMONIC_LENGTH;
        if (position < output_len) {
            output[position++] = ' ';
        }
    }
    PRINTF("SSKR mnemonic phrase:\n %.*s\n", output_len, output);
    return position;
}

unsigned int bolos_ux_bip39_to_sskr_convert(unsigned char *bip39_words_buffer,
                                            unsigned int bip39_words_buffer_length,
                                            unsigned int bip39_onboarding_kind,
                                            unsigned int *group_descriptor,
                                            uint8_t *share_count,
                                            unsigned char *mnemonics,
                                            unsigned int *mnemonics_len) {
    // get seed from bip39 mnemonic
    uint8_t seed_len = bip39_onboarding_kind * 4 / 3;
    uint8_t seed_buffer[seed_len + 1];

    if (bolos_ux_bip39_mnemonic_decode(bip39_words_buffer,
                                       bip39_words_buffer_length,
                                       seed_buffer,
                                       seed_len + 1) == 1) {
        memset(bip39_words_buffer, 0, bip39_words_buffer_length);
        bip39_words_buffer_length = 0;
        size_t groups_len = 1;
        size_t groups_threshold = 1;
        size_t share_len_expected = 0;
        size_t share_count_expected = bolos_ux_sskr_size_get(bip39_onboarding_kind,
                                                             groups_threshold,
                                                             group_descriptor,
                                                             groups_len,
                                                             &share_len_expected);

        size_t share_buffer_len = share_count_expected * share_len_expected;
        uint8_t share_buffer[share_buffer_len];
        size_t share_len = 0;
        *share_count = bolos_ux_sskr_generate(groups_threshold,
                                              group_descriptor,
                                              groups_len,
                                              seed_buffer,
                                              seed_len,
                                              &share_len,
                                              share_buffer,
                                              share_buffer_len,
                                              share_len_expected,
                                              share_count_expected);
        memset(seed_buffer, 0, seed_len);
        if (*share_count > 0) {
            // CBOR Tag #309 is D9 0135
            // CBOR Major type 2 is 0x40
            // (see https://www.rfc-editor.org/rfc/rfc8949#name-major-types)
            uint8_t cbor[] = {0xD9, 0x01, 0x35, 0x40, 0x00};
            size_t cbor_len = sizeof(cbor);
            if (share_len < 24) {
                cbor[3] |= (share_len & 0x1F);
                cbor_len--;
            } else {
                cbor[3] |= 0x18;
                cbor[4] = (uint8_t) share_len;
            }

            uint32_t checksum = 0;
            uint8_t checksum_len = sizeof(checksum);

            size_t cbor_share_crc_buffer_len = cbor_len + share_len + checksum_len;
            uint8_t cbor_share_crc_buffer[cbor_share_crc_buffer_len];

            // mnemonics_len is space separated bytewords of cbor + share + checksum
            *mnemonics_len =
                ((cbor_len + share_len + checksum_len) * (SSKR_MNEMONIC_LENGTH + 1) - 1) *
                *share_count;

            for (uint8_t share = 0; share < *share_count; share++) {
                memcpy(cbor_share_crc_buffer, cbor, cbor_len);
                memcpy(cbor_share_crc_buffer + cbor_len,
                       share_buffer + share_len * share,
                       share_len);
                // TODO
                // During testing cx_crc32_hw() gave an incorrect CRC32 so disabling for now and
                // using own crc32() function instead checksum =
                // cx_crc32_hw_nbo(cbor_share_crc_buffer, cbor_len + share_len);
                checksum = crc32_nbo(cbor_share_crc_buffer, cbor_len + share_len);
                memcpy(cbor_share_crc_buffer + cbor_len + share_len, &checksum, checksum_len);

                if (bolos_ux_sskr_mnemonic_encode(
                        cbor_share_crc_buffer,
                        cbor_share_crc_buffer_len,
                        mnemonics + share * (*mnemonics_len / *share_count),
                        *mnemonics_len / *share_count) < 1) {
                    memset(share_buffer, 0, share_buffer_len);
                    memset(cbor_share_crc_buffer, 0, cbor_share_crc_buffer_len);
                    memset(mnemonics, 0, *mnemonics_len);
                    mnemonics_len = 0;
                    memset(bip39_words_buffer, 0, bip39_words_buffer_length);
                    return 0;
                }
                memset(cbor_share_crc_buffer, 0, cbor_share_crc_buffer_len);
                checksum = 0;
            }
            memset(share_buffer, 0, share_buffer_len);
        }
    }
    memset(bip39_words_buffer, 0, bip39_words_buffer_length);

    return 1;
}
