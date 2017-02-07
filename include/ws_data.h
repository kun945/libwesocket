/*
 * ws_data.h
 *
 * Create by LiangCha<ckmx945@gmail.com> at 2017-01-17 13:27:57
*/

#ifndef WS_DATA_H_ZENFORPA
#define WS_DATA_H_ZENFORPA

#include "ws_util.h"
#include <inttypes.h>

typedef struct {
    uint8_t     fin;
    uint8_t     mask;
    uint8_t     opcode;
    uint8_t     mask_key[4];
    uint8_t     payload_len;
    uint64_t    extend_len;
    uint64_t    len;
    uint8_t     *data;
} ws_data_head_t;


#define WS_OPCODE_CONTINUATION  0x0
#define WS_OPCODE_TEXT          0x1
#define WS_OPCODE_BINARY        0x2
#define WS_OPCODE_CLOSE         0x8
#define WS_OPCODE_PING          0x9
#define WS_OPCODE_PONG          0xA


enum {
    WS_STATUS_NONE,
    WS_STATUS_NORMAL_CLOSURE,
    WS_STATUS_GOING_AWAY,
    WS_STATUS_PROTOCOL_ERROR,
    WS_STATUS_UNSUPPORTED_DATA,
    WS_STATUS_RESERVED,
    WS_STATUS_NO_STATUS,
    WS_STATUS_ABNORMAL_CLOSURE,
    WS_STATUS_INVALID_FRAME,
    WS_STATUS_POLICY_VIOLATION,
    WS_STATUS_TOO_BIG,
    WS_STATUS_MANDATORY_EXT,
    WS_STATUS_SERVER_ERR,
    WS_STATUS_TLS_HANDSHAKE,
};


/*int ws_create_close_frame(uint8_t *buffer, int len, int mask);*/
int ws_create_close_frame(uint8_t *buffer, int len, int status, int mask);
int ws_parse_data_frame(ws_data_head_t *dh, uint8_t *buffer, uint64_t len);
int ws_create_data_frame(ws_data_head_t *dh, uint8_t *buffer, uint64_t len);
int ws_create_text_frame(uint8_t *buffer, uint64_t len, const char *text,
        int mask);
int ws_create_binary_frame(uint8_t *buffer, uint64_t len, void  *bin,
        uint64_t bin_len, int mask);
int ws_create_ping_frame(uint8_t *buffer, uint64_t len, void  *data,
        uint64_t data_len, int mask);
int ws_create_pong_frame(uint8_t *buffer, uint64_t len, void  *data,
        uint64_t data_len, int mask);
int ws_create_continuation_frame(uint8_t *buffer, uint64_t len, void *data,
        uint64_t data_len, int mask);

/*
 * @return 0 a complete frame, 1 determined data frame, -1 error
 * */
int ws_parse_data_frame_stream(ws_data_head_t *dh, uint8_t *buffer,
        uint64_t len, uint8_t **pos);

#endif /* end of include guard: WS_DATA_H_ZENFORPA */

