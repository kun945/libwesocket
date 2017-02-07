/*
 * ws_data.c
 *
 * Create by LiangCha<ckmx945@gmail.com> at 2017-01-17 13:27:31
*/

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "ws_data.h"


struct stcode {
    int     code;
    char    meaning[64];
};

enum {
    _STCODE_NONE,
    _STCODE_NORMAL_CLOSURE = 1000,
    _STCODE_GOING_AWAY,       /* 1001 */
    _STCODE_PROTOCOL_ERROR,   /* 1002 */
    _STCODE_UNSUPPORTED_DATA, /* 1003 */
    _STCODE_RESERVED,         /* 1004 */
    _STCODE_NO_STATUS,        /* 1005 */
    _STCODE_ABNORMAL_CLOSURE, /* 1006 */
    _STCODE_INVALID_FRAME,    /* 1007 */
    _STCODE_POLICY_VIOLATION, /* 1008 */
    _STCODE_TOO_BIG,          /* 1009 */
    _STCODE_MANDATORY_EXT,    /* 1010 */
    _STCODE_SERVER_ERR,       /* 1011 */
    _STCODE_TLS_HANDSHAKE = 1015,    /* 1015 */
};

static struct stcode _st_table[] = {
    {_STCODE_NONE,    ""},
    {_STCODE_NORMAL_CLOSURE,      "Normal Closure"},
    {_STCODE_GOING_AWAY,          "Going Away"},
    {_STCODE_PROTOCOL_ERROR,      "Protocol error"},
    {_STCODE_UNSUPPORTED_DATA,    "Unsupported Data"},
    {_STCODE_RESERVED,            "Reserved"},
    {_STCODE_NO_STATUS,           "No Status Rcvd"},
    {_STCODE_ABNORMAL_CLOSURE,    "Abnormal Closure"},
    {_STCODE_INVALID_FRAME,       "Invalid frame payload data"},
    {_STCODE_POLICY_VIOLATION,    "Policy Violation"},
    {_STCODE_TOO_BIG,             "Message Too Big"},
    {_STCODE_MANDATORY_EXT,       "Mandatory Ext"},
    {_STCODE_SERVER_ERR,          "Internal Server Err"},
    {_STCODE_TLS_HANDSHAKE,       "TLS handshake"},
};


static uint32_t _seed = 5201314;

static void _fill_mask_key(uint8_t *buffer)
{
    srand(_seed);
    uint32_t key = rand() % 0xFFFFFFFF;
    *((uint32_t*)buffer) = key;
    _seed = (_seed + key) * 1103515245;
}


static void _adjust_length(ws_data_head_t *dh, uint64_t len)
{
    if (len > 0xFFFF) {
        dh->payload_len = 127;
        dh->extend_len = len;
    } else if (len >= 126) {
        dh->payload_len  = 126;
        dh->extend_len = len;
    } else {
        dh->payload_len = len;
    }
    dh->len = len;
}


int ws_parse_data_frame(ws_data_head_t *dh, uint8_t *buffer, uint64_t len)
{
    if (len < 2) {
        return -1;
    }

    dh->fin = buffer[0] & 0x80 ? 1 : 0;
    dh->opcode = (buffer[0] & 0xF);
    dh->mask = buffer[1] & 0x80 ? 1 : 0;
    dh->payload_len = buffer[1] & 0x7F;

    uint64_t index = 2;
    if (dh->payload_len < 126) {
        dh->len =  dh->payload_len;
    } else if (dh->payload_len == 126 && len >= 6) {
        dh->extend_len = ws_ntoh16(buffer + index);
        dh->len = dh->extend_len;
        index += 2;
    } else if (dh->payload_len == 127 && len >= 10) {
        dh->extend_len = ws_ntoh64(buffer + index);
        dh->len = dh->extend_len;
        index += 8;
    } else {
        return -1;
    }

    if (dh->mask) {
        if ((index + 4) > len) {
            return -1;
        }
        memcpy(dh->mask_key, buffer + index, sizeof(dh->mask_key));
        index += 4;
    }

    dh->data = buffer + index;
    if ((len - index) < dh->len) {
        return -1;
    }
    return 0;
}


int ws_create_close_frame(uint8_t *buffer, int len, int status, int mask)
{
    ws_data_head_t dh;
    memset(&dh, 0, sizeof(dh));
    dh.fin = 1;
    dh.opcode = WS_OPCODE_CLOSE;

    int data_len = 0;
    uint8_t data[128];
    if (status != WS_STATUS_NONE) {
        *((uint16_t*)data) = ws_hton16(_st_table[status].code);
        int cpy_len = strlen(_st_table[status].meaning);
        data_len = (cpy_len + 2) > 128 ? 128 : cpy_len + 2;
        memcpy(data + 2, _st_table[status].meaning, data_len - 2);
    }
    dh.data = data;
    dh.mask = mask;
    _adjust_length(&dh, data_len);
    return ws_create_data_frame(&dh, buffer, len);
}


/*int ws_create_close_frame(uint8_t *buffer, int len, int mask)*/
/*{*/
    /*if (len < 2 || (mask && len < 6)) {*/
        /*return -1;*/
    /*}*/

    /*int index = 2;*/
    /*buffer[0] = 0x88; [> FIN is ture, opencode is 0x8<]*/
    /*if (mask) {*/
        /*buffer[1] = 0x1 << 7;*/
        /*_fill_mask_key(buffer + 2);*/
        /*index += 4;*/
    /*} else {*/
        /*buffer[1] = 0;*/
    /*}*/

    /*return index;*/
/*}*/


int ws_create_data_frame(ws_data_head_t *dh, uint8_t *buffer, uint64_t len)
{
    if (dh->fin == 1) {
        buffer[0] = (0x80) + (dh->opcode);
    } else if (dh->fin == 0) {
        buffer[0] = 0;
    } else {
        return -1;
    }

    uint64_t index = 2;
    if (dh->payload_len < 126) {
        ;
    } else if (dh->payload_len == 126 && len >= 6) {
        *((uint16_t *)(buffer + index)) = ws_hton16(dh->extend_len);
        index += 2;
    } else if (dh->payload_len == 127 && len >= 10) {
        *((uint64_t*)(buffer + index)) = ws_hton64(dh->extend_len);
        index += 8;
    } else {
        return -1;
    }
    buffer[1] = dh->payload_len;

    uint64_t key_index = index;
    if (dh->mask == 1) {
        if ((index + 4) > len) {
            return -1;
        }
        buffer[1] += (0x1 << 7);
        _fill_mask_key(buffer + index);
        index += 4;
    }

    if ((len - index) < dh->len) {
        return -1;
    }

    memcpy(buffer + index, dh->data, dh->len);
    if (dh->mask) {
        ws_data_mask(buffer + index, dh->len, buffer + key_index);
    }
    return dh->len + index;
}


int ws_create_text_frame(uint8_t *buffer, uint64_t len, const char *text,
        int mask)
{
    ws_data_head_t dh;
    memset(&dh, 0, sizeof(dh));
    dh.fin = 1;
    dh.opcode = WS_OPCODE_TEXT;
    dh.data = (uint8_t *)text;
    dh.mask = mask;
    _adjust_length(&dh, strlen(text));
    return ws_create_data_frame(&dh, buffer, len);
}


int ws_create_binary_frame(uint8_t *buffer, uint64_t len, void *bin,
        uint64_t bin_len, int mask)
{
    ws_data_head_t dh;
    memset(&dh, 0, sizeof(dh));
    dh.fin = 1;
    dh.opcode = WS_OPCODE_BINARY;
    _adjust_length(&dh, bin_len);
    dh.data = bin;
    dh.mask = mask;
    return ws_create_data_frame(&dh, buffer, len);
}


int ws_create_ping_frame(uint8_t *buffer, uint64_t len, void *data,
        uint64_t data_len, int mask)
{
    ws_data_head_t dh;
    memset(&dh, 0, sizeof(dh));
    dh.fin = 1;
    dh.opcode = WS_OPCODE_PING;
    _adjust_length(&dh, data_len);
    dh.data = data;
    dh.mask = mask;
    return ws_create_data_frame(&dh, buffer, len);
}


int ws_create_pong_frame(uint8_t *buffer, uint64_t len, void *data,
        uint64_t data_len, int mask)
{
    ws_data_head_t dh;
    memset(&dh, 0, sizeof(dh));
    dh.fin = 1;
    dh.opcode = WS_OPCODE_PONG;
    _adjust_length(&dh, data_len);
    dh.data = data;
    dh.mask = mask;
    return ws_create_data_frame(&dh, buffer, len);
}


int ws_create_continuation_frame(uint8_t *buffer, uint64_t len, void *data,
        uint64_t data_len, int mask)
{
    ws_data_head_t dh;
    memset(&dh, 0, sizeof(dh));
    dh.fin = 0;
    dh.opcode = WS_OPCODE_CONTINUATION;
    _adjust_length(&dh, data_len);
    dh.data = data;
    dh.mask = mask;
    return ws_create_data_frame(&dh, buffer, len);
}


int ws_parse_data_frame_stream(ws_data_head_t *dh, uint8_t *buffer,
        uint64_t len, uint8_t **pos)
{
    uint64_t index = 0;
    if (len < 2) {
        *pos = buffer;
        return 1;
    }

    if ((0x70 & buffer[0])) { /* rsv must be 0 */
        *pos = buffer;
        return -1;
    }

    dh->fin = buffer[0] & 0x80 ? 1 : 0;
    dh->opcode = (buffer[0] & 0xF);
    dh->mask = buffer[1] & 0x80 ? 1 : 0;
    dh->payload_len = buffer[1] & 0x7F;
    index += 2;

    if (dh->payload_len < 126) {
        dh->len =  dh->payload_len;
    } else if (dh->payload_len == 126 && len >= 6) {
        if (len < (2 + index)) {
            goto determined;
        }
        dh->extend_len = ws_ntoh16(buffer + index);
        dh->len = dh->extend_len;
        index += 2;
    } else if (dh->payload_len == 127 && len >= 10) {
        if (len < (8 + index)) {
            goto determined;
        }
        dh->extend_len = ws_ntoh64(buffer + index);
        dh->len = dh->extend_len;
        index += 8;
    } else {
        *pos = buffer;
        return -1;
    }

    if (dh->mask) {
        if ((index + 4) > len) {
            goto determined;
        }
        memcpy(dh->mask_key, buffer + index, sizeof(dh->mask_key));
        index += 4;
    }

    dh->data = buffer + index;
    if ((len - index) < dh->len) {
        goto determined;
    }

    *pos = buffer + index + dh->len;
    return 0;

determined :
    *pos = buffer + len;
    return 1;
}
