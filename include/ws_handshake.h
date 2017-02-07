/*
 * ws_handshake.h
 *
 * Create by LiangCha<ckmx945@gmail.com> at 2017-01-16 15:06:34
*/


#ifndef WS_HANDSHAKE_H_1QYSQK3G
#define WS_HANDSHAKE_H_1QYSQK3G

#define WS_RESPONSE_LEN             1024
#define WS_REQUEST_LEN              1024

typedef struct ws_request {
    int             version;
    int             protocol;
    char            *version_str;
    char            *connection;
    char            *key;
    char            *origin;
    char            *upgrade;
    char            *get;
    char            *accept;
    char            *extension;
    char            *protocol_str;
    char            *host;
    char            *uri;
    char            *http_version;
    char            response[WS_RESPONSE_LEN];
    char            request[WS_REQUEST_LEN];
} ws_request_t;


#define WS_REQUEST_GET_STR          "GET "
#define WS_REQUEST_GET_LEN          4

#define WS_REQUEST_HOST_STR         "Host: "
#define WS_REQUEST_HOST_LEN         6

#define WS_REQUEST_UPGRADE_STR      "Upgrade: "
#define WS_REQUEST_UPGRADE_LEN      9

#define WS_REQUEST_CONNECTION_STR   "Connection: "
#define WS_REQUEST_CONNECTION_LEN   12

#define WS_REQUEST_KEY_STR          "Sec-WebSocket-Key: "
#define WS_REQUEST_KEY_LEN          19

#define WS_REQUEST_ORIGIN_STR       "Origin: "
#define WS_REQUEST_ORIGIN_LEN       8

#define WS_REQUEST_PROTOCOL_STR     "Sec-WebSocket-Protocol: "
#define WS_REQUEST_PROTOCOL_LEN     24

#define WS_REQUEST_VERSION_STR      "Sec-WebSocket-Version: "
#define WS_REQUEST_VERSION_LEN      23

enum {
    WS_PROTOCOL_UNKNOW,
    WS_PROTOCOL_CHAT,
    WS_PROTOCOL_ECHO
};


int ws_parse_request(ws_request_t *req, char *req_str);

#endif /* end of include guard: WS_HANDSHAKE_H_1QYSQK3G */

