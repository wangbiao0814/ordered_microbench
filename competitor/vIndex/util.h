#ifndef _UTIL_H_
#define _UTIL_H_

#include <cstdint>

typedef uint64_t KEY_TYPE;
typedef uint64_t VALUE_TYPE;


#define KEY_BYTES (sizeof(KEY_TYPE))
#define VALUE_BYTES (sizeof(VALUE_TYPE))

static uint8_t byte_count[256] = {
                                    0,1,2,3,1,2,3,4,2,3,4,5,3,4,5,6,
                                    1,2,3,4,2,3,4,5,3,4,5,6,4,5,6,7,
                                    2,3,4,5,3,4,5,6,4,5,6,7,5,6,7,8,
                                    3,4,5,6,4,5,6,7,5,6,7,8,6,7,8,9,
                                    1,2,3,4,2,3,4,5,3,4,5,6,4,5,6,7,
                                    2,3,4,5,3,4,5,6,4,5,6,7,5,6,7,8,
                                    3,4,5,6,4,5,6,7,5,6,7,8,6,7,8,9,
                                    4,5,6,7,5,6,7,8,6,7,8,9,7,8,9,10,
                                    2,3,4,5,3,4,5,6,4,5,6,7,5,6,7,8,
                                    3,4,5,6,4,5,6,7,5,6,7,8,6,7,8,9,
                                    4,5,6,7,5,6,7,8,6,7,8,9,7,8,9,10,
                                    5,6,7,8,6,7,8,9,7,8,9,10,8,9,10,11,
                                    3,4,5,6,4,5,6,7,5,6,7,8,6,7,8,9,
                                    4,5,6,7,5,6,7,8,6,7,8,9,7,8,9,10,
                                    5,6,7,8,6,7,8,9,7,8,9,10,8,9,10,11,
                                    6,7,8,9,7,8,9,10,8,9,10,11,9,10,11,12};


inline uint8_t ctz_16(uint16_t x)
{
    uint8_t n = 1;
    if((x & 0xFF) == 0) {n += 8; x >>= 8;}
    if((x & 0x0F) == 0) {n += 4; x >>= 4;}
    if((x & 0x03) == 0) {n += 2; x >>= 2;}
    return n - (x & 1);
}

inline uint8_t ctz_32(uint32_t x)
{
    uint8_t n = 1;
    if((x & 0xFFFF) == 0) {n += 16; x >>= 16;}
    if((x & 0x00FF) == 0) {n += 8; x >>= 8;}
    if((x & 0x000F) == 0) {n += 4; x >>= 4;}
    if((x & 0x0003) == 0) {n += 2; x >>= 2;}
    return n - (x & 1);
}

inline uint8_t ctz_64(uint64_t x)
{
    uint8_t n = 1;
    if((x & 0xFFFFFFFF) == 0) {n += 32; x >>= 32;}
    if((x & 0x0000FFFF) == 0) {n += 16; x >>= 16;}
    if((x & 0x000000FF) == 0) {n += 8; x >>= 8;}
    if((x & 0x0000000F) == 0) {n += 4; x >>= 4;}
    if((x & 0x00000003) == 0) {n += 2; x >>= 2;}
    return n - (x & 1);
}

inline bool isUpperLevel(int level)
{
    return (level & 1) ? false : true;
}

#endif