#include "ws_data.h"
#include <stdio.h>
#include <string.h>

static uint8_t _abc_frame[] = {0x81, 0x83, 0xb7, 0xe7, 0x44, 0x7c, 0xd6, 0x85, 0x27};
static uint8_t _close_frame[] = {0x88, 0x80, 0x68, 0x67, 0xd1, 0x21};

int main (void)
{
    int ret;
    uint8_t a[1024];
    ws_data_head_t dh;
    printf("-----------------------------------------------------\n");
    int i;
    for (i = 0; i < 100; ++i) {
        uint8_t *pos;
        memset(&dh, 0, sizeof(dh));
        ret = ws_parse_data_frame_stream(&dh, _abc_frame, i, &pos);
        printf("ws_parse_data_frame, %d\n", ret);
        if (ret == 0) {
            memcpy(a, dh.data, dh.len);
            ws_data_mask(dh.data, dh.len, (uint8_t*)&dh.mask_key);
            uint64_t i;
            printf("data: ");
            for (i = 0; i < dh.len; ++i) {
                printf("%02X, ", dh.data[i]);
            }
            printf("\n");
            printf("mask %d, fin %d, mask_key %08X, opencode %02X\n", dh.mask, dh.fin, *((int32_t*)dh.mask_key), dh.opcode);
            break;
        } else if (ret == 1) {
            printf("determined 0x%llX\n", (unsigned long long)pos);
        } else {
            printf("error %d\n", ret);
        }
    }
    return 0;
}
