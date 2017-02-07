#include "ws_data.h"
#include <stdio.h>
#include <string.h>

static uint8_t _abc_frame[] = {0x81, 0x83, 0xb7, 0xe7, 0x44, 0x7c, 0xd6, 0x85, 0x27};
static uint8_t _close_frame[] = {0x88, 0x80, 0x68, 0x67, 0xd1, 0x21};

int main (void)
{
    uint8_t buffer[1024];
    ws_data_head_t dh;
    printf("text -------------------------------------------------\n");
    int len = ws_create_text_frame(buffer,  1024, "hehe", 1);
    printf("ws_create_data_frame, %d\n", len);
    if (len > 0) {
        int i;
        printf("frame: ");
        for (i = 0; i < len; ++i) {
            printf("%02X, ", buffer[i]);
        }
        printf("\n");
    }

    uint8_t a[1024];
    memset(&dh, 0, sizeof(dh));
    int ret = ws_parse_data_frame(&dh, buffer, len);
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
    }

    uint8_t test_bin[] = {0x1, 0x2, 0x3};
    printf("bin  -------------------------------------------------\n");
    len = ws_create_binary_frame(buffer,  1024, test_bin, 3, 1);
    printf("ws_create_data_frame, %d\n", len);
    if (len > 0) {
        int i;
        printf("frame: ");
        for (i = 0; i < len; ++i) {
            printf("%02X, ", buffer[i]);
        }
        printf("\n");
    }

    memset(&dh, 0, sizeof(dh));
    ret = ws_parse_data_frame(&dh, buffer, len);
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
    }

    printf("pong-------------------------------------------------\n");
    len = ws_create_pong_frame(buffer,  1024, (uint8_t *)"hehe", strlen("hehe"), 1);
    printf("ws_create_data_frame, %d\n", len);
    if (len > 0) {
        int i;
        printf("frame: ");
        for (i = 0; i < len; ++i) {
            printf("%02X, ", buffer[i]);
        }
        printf("\n");
    }

    memset(&dh, 0, sizeof(dh));
    ret = ws_parse_data_frame(&dh, buffer, len);
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
    }

    printf("ping-------------------------------------------------\n");
    len = ws_create_ping_frame(buffer,  1024, (uint8_t *)"hehe", strlen("hehe"), 1);
    printf("ws_create_data_frame, %d\n", len);
    if (len > 0) {
        int i;
        printf("frame: ");
        for (i = 0; i < len; ++i) {
            printf("%02X, ", buffer[i]);
        }
        printf("\n");
    }

    memset(&dh, 0, sizeof(dh));
    ret = ws_parse_data_frame(&dh, buffer, len);
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
    }

    printf("-----------------------------------------------------\n");
    memset(&dh, 0, sizeof(dh));
    ret = ws_parse_data_frame(&dh, _abc_frame, sizeof(_abc_frame));
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
    }

    memset(&dh, 0, sizeof(dh));
    ret = ws_parse_data_frame(&dh, _close_frame, sizeof(_abc_frame));
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
    }

    printf("close------------------------------------------------\n");
    len = ws_create_close_frame(buffer, 1024, 0, 1);
    printf("ws_create_data_frame, %d\n", len);
    if (len > 0) {
        int i;
        printf("frame: ");
        for (i = 0; i < len; ++i) {
            printf("%02X, ", buffer[i]);
        }
        printf("\n");
    }

    memset(&dh, 0, sizeof(dh));
    ret = ws_parse_data_frame(&dh, buffer, len);
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
    }

    return 0;
}
