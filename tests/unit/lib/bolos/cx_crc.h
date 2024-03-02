#pragma once

#include <stddef.h>

unsigned short cx_crc16_update(unsigned short crc, const void *buf,
                               size_t len);

unsigned int cx_crc32_update(unsigned int crc, const void *buf, size_t len);
