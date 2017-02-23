#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ws_handshake.h"


int main (int argc, char *argv[])
{
    char a[] = "ws://192.168.1.1/";
    char b[] = "ws://192.168.1.1:8080";
    char c[] = "ws://192.168.1.1/a/b/c/";
    char d[] = "ws://test.xiahuo.top/a/b/c/";
    char e[] = "ws://test.xiahuo.top:7907/a/b/c/";

    ws_request_ctx_t req;

    printf("a----------------------------------------\n");
    ws_create_request(&req, a);
    printf("host %s, port %d, key %s, accept_key %s\n",
            req.host, req.port, req.request_key, req.accept_key);
    printf("%s", req.send_str);

    printf("b----------------------------------------\n");
    ws_create_request(&req, b);
    printf("host %s, port %d, key %s, accept_key %s\n",
            req.host, req.port, req.request_key, req.accept_key);
    printf("%s", req.send_str);

    printf("c----------------------------------------\n");
    ws_create_request(&req, c);
    printf("host %s, port %d, key %s, accept_key %s\n",
            req.host, req.port, req.request_key, req.accept_key);
    printf("%s", req.send_str);

    printf("d----------------------------------------\n");
    ws_create_request(&req, d);
    printf("host %s, port %d, key %s, accept_key %s\n",
            req.host, req.port, req.request_key, req.accept_key);
    printf("%s", req.send_str);

    printf("e----------------------------------------\n");
    ws_create_request(&req, e);
    printf("host %s, port %d, key %s, accept_key %s\n",
            req.host, req.port, req.request_key, req.accept_key);
    printf("%s", req.send_str);

    return 0;
}
