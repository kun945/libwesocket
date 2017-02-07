/*
 * util.h
 *
 * Create by LiangCha<ckmx945@gmail.com> at 2017-01-17 15:13:27
*/
#ifndef WS_UTIL_H_8SXZ6LEU
#define WS_UTIL_H_8SXZ6LEU

#include <inttypes.h>

static inline uint16_t ws_ntoh16(uint8_t *data)
{
    return ((data[0] << 8) + data[1]);
}


static inline uint64_t ws_ntoh64(uint8_t *data)
{
    int i;
    uint64_t temp = 0;
    for (i = 0; i < 8; ++i) {
        temp += (data[i] << ((7 - i) * 8));
    }

    return temp;
}

static inline uint16_t ws_hton16(uint16_t data)
{
    return ((data & 0xFF00) >> 8) + ((data & 0xFF) << 8);
}

static inline uint64_t ws_hton64(uint64_t data)
{
    int i;
    uint8_t *temp = (uint8_t *)&data;
    uint64_t sum = 0;
    for (i = 0; i < 8; ++i) {
       sum += (temp[i] << ((7 - i ) * 8));
    }

    return 0;
}


static void ws_data_mask(uint8_t *data, uint64_t len, uint8_t *mask_key)
{
    int i;
    for (i = 0; len > 0 ; len--, i++) {
        data[i] = data[i] ^ mask_key[i % 4];
    }
}

#endif /* end of include guard: WS_UTIL_H_8SXZ6LEU */
