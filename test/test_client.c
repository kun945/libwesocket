#include "ws_handshake.h"
#include "ws_data.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ev.h>
#include <sys/types.h> 			/* socket, setsockopt, accept, send, recv */
#include <sys/socket.h> 		/* socket, setsockopt, inet_ntoa, accept */
#include <netinet/in.h> 		/* sockaddr_in, inet_ntoa */
#include <arpa/inet.h> 			/* htonl, htons, inet_ntoa */
#include <sys/stat.h> 			/* stat */
#include <sys/fcntl.h>

typedef struct ws_buffer {
    unsigned size;
    unsigned pos;
    uint8_t *data;      /* buffer start address*/
} ws_buffer_t;


typedef struct ws_client {
    int                 fd;
    ev_io               recv_w;
    ws_buffer_t         buf;
    ws_request_ctx_t    req;
} ws_client_t;


static void _usage(const char *name)
{
    printf("%s <uri>\n", name);
    exit(0);
}


static int _set_noblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        perror("fcntl(F_GETFL)");
        return -1;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        perror("fcntl(F_SETFL)");
        return -1;
    }

    return 0;
}


static ws_client_t * _new_client(const char *uri)
{
    ws_client_t * wc = malloc(sizeof(ws_client_t));
    if (NULL == wc) {
        perror("malloc()");
        return NULL;
    }

    if (ws_create_request(&wc->req, uri) != 0) {
        printf("ws_create_request() failed\n");
        goto free_mem;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        goto free_mem;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(wc->req.host);
    addr.sin_port = htons(wc->req.port);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        perror("connect");
        goto close_fd;
    }

    if (_set_noblock(fd) != 0) {
        perror("_set_noblock");
        goto close_fd;
    }


    if (write(fd, wc->req.send_str, strlen(wc->req.send_str)) <= 0) {
        perror("write");
        goto close_fd;
    }

    wc->fd = fd;
    return wc;

close_fd:
    close(fd);

free_mem:
    free(wc);
    return NULL;
}


static void _del_client(ws_client_t *wc)
{
    close(wc->fd);
    free(wc);
}


static void _recv_data(EV_P, ev_io *w, int revent)
{
    if (revent & EV_ERROR) {
        printf("_recv_data error\n");
        return;
    }

    ws_client_t *wc = w->data;
    printf(" 1 buffer size=%d, pos=%d\n", wc->buf.size, wc->buf.pos);
    if (wc->buf.pos == wc->buf.size) {
        wc->buf.data = realloc(wc->buf.data, wc->buf.size *=2);
        if (wc->buf.data == NULL) {
            perror("realloc");
            goto close_client;
        }

    }
    printf(" 2 buffer size=%d, pos=%d\n", wc->buf.size, wc->buf.pos);

    int recv_len = read(w->fd, wc->buf.data + wc->buf.pos,
            wc->buf.size - wc->buf.pos);
    if (0 == recv_len) {
        goto close_client;
    } else if (recv_len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        } else {
            perror("read()");
            goto close_client;
        }
    }

    wc->buf.pos += recv_len;
    printf("3 buffer size=%d, pos=%d\n", wc->buf.size, wc->buf.pos);

    uint8_t *pos;
    ws_data_head_t dh;

    while (wc->buf.pos > 0) {
    int ret = ws_parse_data_frame_stream(&dh, wc->buf.data, wc->buf.pos, &pos);
    printf("ws_parse_data_frame, %d\n", ret);
    if (ret == 0) {
        ws_data_mask(dh.data, dh.len, (uint8_t*)&dh.mask_key);
        uint64_t i;
        printf("data: ");
        for (i = 0; i < dh.len; ++i) {
            printf("%02X, ", dh.data[i]);
        }

        printf("\n");
        printf("mask %d, fin %d, mask_key %08X, opencode %02X, len %llu\n",
                dh.mask, dh.fin, *((int32_t*)dh.mask_key), dh.opcode, dh.len);

        char s[1024];
        memcpy(s, dh.data, dh.len < 900 ? dh.len : 900);
        s[dh.len < 900 ? dh.len : 900] = 0;
        int send_len = 0;
        uint8_t temp[1024];
        if (dh.opcode == WS_OPCODE_TEXT) {
            send_len = ws_create_text_frame(temp, 1024, s, 0);
            write(w->fd, temp, send_len);
            /*send_len = ws_create_continuation_frame(temp, 1024, "hehe", 4, 0);*/
            /*write(w->fd, temp, send_len);*/
        } else if (dh.opcode == WS_OPCODE_CLOSE) {
            send_len = ws_create_close_frame(temp, 1024, 0, 0);
            write(w->fd, temp, send_len);
            goto close_client;
        } else if (dh.opcode == WS_OPCODE_PING) {
            printf("ping not support opcode %02X\n", dh.opcode);
        } else if (dh.opcode == WS_OPCODE_PONG) {
            printf("pong not support opcode %02X\n", dh.opcode);
        } else if (dh.opcode == WS_OPCODE_BINARY) {
            printf("binary not support opcode %02X\n", dh.opcode);
        } else if (dh.opcode == WS_OPCODE_CONTINUATION) {
            printf("continue not support opcode %02X\n", dh.opcode);
        } else {
            printf("not support opcode %02X\n", dh.opcode);
            goto close_client;
        }
        memmove(wc->buf.data, pos, wc->buf.pos - (pos - wc->buf.data));
        wc->buf.pos = wc->buf.pos - (pos - wc->buf.data);
        printf("4 buffer size=%d, pos=%d\n", wc->buf.size, wc->buf.pos);
    } else if (ret == 1) {
        printf("incomplete data frame\n");
        return;
    } else {
        printf("not support opcode %02X\n", dh.opcode);
        goto close_client;
    }
    }

    return;

close_client:
    printf("%s:%d connection will be close\n", wc->req.host, wc->req.port);
    ev_io_stop(loop, &wc->recv_w);
    _del_client(wc);
    return;

}


static void _recv_shake_hand(EV_P, ev_io *w, int revent)
{
    if (revent & EV_ERROR) {
        printf("_recv_shake_hand error\n");
        return;
    }

    ws_client_t *wc = w->data;
    char buffer[1024];
    int recv_len = read(w->fd, buffer, 1024);
    if (0 == recv_len) {
        goto close_client;
    } else if (recv_len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        } else {
            perror("read()");
            goto close_client;
        }
    }

    int ret = ws_parse_response(&wc->req, buffer);
    if (ret != 0) {
        printf("ws_parse_response failed, %d\n", wc->req.status_code);
    } else {
        if ((wc->buf.data = malloc(128)) == NULL) {
            perror("malloc");
        }
        wc->buf.size = 128;
        wc->buf.pos = 0;
        ev_io_stop(loop, w);
        ev_io_init(w, _recv_data, w->fd, EV_READ);
        ev_io_start(loop, w);
        char a[] = {0x11, 0x22, 0x33};
        int send_len = ws_create_ping_frame((uint8_t*)buffer, 1024, a, 3, 1);
        write(wc->fd, buffer, send_len);
    }

close_client:
    printf("%s:%d connection will be close\n", wc->req.host, wc->req.port);
    ev_io_stop(loop, &wc->recv_w);
    _del_client(wc);
    return;
}


int main (int argc, char *argv[]) {

    if (argc != 2) {
        _usage(argv[0]);
    }

    ws_client_t *wc = _new_client(argv[1]);
    if (NULL == wc) {
        return -1;
    }

    struct ev_loop *loop = ev_loop_new(0);
    ev_io_init(&wc->recv_w, _recv_shake_hand, wc->fd, EV_READ);
    wc->recv_w.data = wc;
    ev_io_start(loop, &wc->recv_w);
    ev_run(loop, 0);

    return 0;
}
