#include <stdint.h>
#include <stddef.h>

// Fake random generator used only for testing
void cx_rng_no_throw(uint8_t *buffer, size_t len)
{
    for (size_t i = 0; i < len; i++) {
       buffer[i] = (uint8_t) i;
    } 
}

// Fake secure memory comparison used only for testing
char os_secure_memcmp(const void *src1, const void *src2, size_t length)
{
#define SRC1 ((unsigned char const *) src1)
#define SRC2 ((unsigned char const *) src2)
    unsigned int  l      = length;
    unsigned char xoracc = 0;
    // don't || to ensure all condition are evaluated
    while (!(!length && !l)) {
        length--;
        xoracc |= SRC1[length] ^ SRC2[length];
        l--;
    }
    return xoracc;
}

uint32_t cx_crc32_hw(const uint8_t *data, size_t len) {
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
