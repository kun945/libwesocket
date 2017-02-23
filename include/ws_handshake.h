/*
 * ws_handshake.h
 *
 * Create by LiangCha<ckmx945@gmail.com> at 2017-01-16 15:06:34
*/


#ifndef WS_HANDSHAKE_H_1QYSQK3G
#define WS_HANDSHAKE_H_1QYSQK3G

#define WS_RESPONSE_LEN             1024
#define WS_REQUEST_LEN              1024

typedef struct ws_handshake_ctx {
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
    /*char            recv_str[WS_RESPONSE_LEN];*/
    char            send_str[WS_RESPONSE_LEN]; /* this buffer, used to create the response */
} ws_handshake_ctx_t;


typedef struct ws_request_ctx {
    int             port;
    int             status_code;
    char            host[32];
    char            request_key[64];
    char            accept_key[64];
    char            send_str[WS_REQUEST_LEN];
} ws_request_ctx_t;


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

#define WS_URI_PREFIX_STR           "ws://"
#define WS_URI_PREFIX_LEN           5

#define WS_RESPONSE_VERSION_STR     "HTTP/1.1 "
#define WS_RESPONSE_VERSION_LEN     9

#define WS_RESPONSE_UPGRADE_STR     WS_REQUEST_UPGRADE_STR
#define WS_RESPONSE_UPGRADE_LEN     WS_REQUEST_UPGRADE_LEN

#define WS_RESPONSE_CONNECTION_STR  WS_REQUEST_CONNECTION_STR
#define WS_RESPONSE_CONNECTION_LEN  WS_REQUEST_CONNECTION_LEN

#define WS_RESPONSE_PROTOCOL_STR    WS_REQUEST_PROTOCOL_STR
#define WS_RESPONSE_PROTOCOL_LEN    WS_REQUEST_PROTOCOL_LEN

#define WS_RESPONSE_ACCEPT_STR      "Sec-WebSocket-Accept: "
#define WS_RESPONSE_ACCEPT_LEN      22


enum {
    WS_PROTOCOL_UNKNOW,
    WS_PROTOCOL_CHAT,
    WS_PROTOCOL_ECHO
};


int ws_parse_request(ws_handshake_ctx_t *req, char *req_str);
int ws_create_request(ws_request_ctx_t *req, const char *uri);
int ws_parse_response(ws_request_ctx_t *req, char *resp_str);

#endif /* end of include guard: WS_HANDSHAKE_H_1QYSQK3G */

