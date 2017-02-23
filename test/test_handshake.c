#include "ws_handshake.h"
#include "stdio.h"
#include <string.h>

char test[] = "GET /chat HTTP/1.1\r\n"
    "Host: server.example.com\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
    "Origin: http://example.com\r\n"
    "Sec-WebSocket-Protocol: chat, superchat\r\n"
    "Sec-WebSocket-Version: 13\r\n\r\n";

int main (void) {
    ws_handshake_ctx_t r;
    memset(&r, 0, sizeof(0));

    int ret;
    if ((ret = ws_parse_request(&r, test)) != 0) {
        printf("ws_parse_request filed, %d\n", ret);
    }

    printf("%s\n", test);
    printf("%s\n", r.send_str);

    return 0;
}
