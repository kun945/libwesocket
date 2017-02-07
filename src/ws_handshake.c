/*
 * ws_handshake.c
 *
 *
 * Create by LiangCha<ckmx945@gmail.com> at 2017-01-16 15:35:33
*/

#include <stdio.h>
#include "stdlib.h"
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "md5.h"
#include "sha1.h"
#include "base64.h"
#include "ws_handshake.h"

#define _HTTP_VERSION_STR       "HTTP/1.1"
#define _UPGRADE_STR            "websocket"
#define _CONNECTION_STR         "Upgrade"

static char *_status_lines[] = {
    "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n", /* wesocket handshake request correct */
    "HTTP/1.1 400 Bad Request\r\n\r\n",
    "HTTP/1.1 403 Forbidden\r\n\r\n",
    "HTTP/1.1 426 Upgrade Required\r\nSec-WebSocket-Version: 13\r\n\r\n",
    "HTTP/1.1 500 Internal Error\r\n\r\n",
    "HTTP/1.1 501 Not Implemented\r\n\r\n"
};

static char *_accept_protocol[] = {
    "unknow",
    "chat",
    "echo",
};


enum {
    _VALID_REQUEST,
    _BAD_REQUEST,
    _FORBIDDEN,
    _UPGRADE_REQUIRED,
    _INTERNAL_ERROR,
    _NOT_IMPLEMENTED
};


static int _parse_get(char *get, char **uri, char **http_version)
{
    char *temp;
    *uri = get + WS_REQUEST_GET_LEN;
    if ((*uri)[0] != '/') {
        return -1;
    }
    temp = strchr((*uri), ' ');
    if (NULL == temp) {
        return -1;
    }

    *temp = '\0';
    *http_version = temp + 1;
    return 0;
}

static char * _create_accept_key(const char *key)
{
    SHA1Context sha;
    char* magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    int i;
    uint32_t number;
    char sha1_key[20];
    char temp[128];

    snprintf(temp, sizeof(temp), "%s%s", key, magic);
    SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char*) temp, strlen(temp));

    if ( !SHA1Result(&sha) ) {
        return NULL;
    }

    for(i = 0; i < 5; i++) {
        number = ntohl(sha.Message_Digest[i]);
        memcpy(sha1_key+(4*i), (unsigned char *) &number, 4);
    }

    char *accept_key;
    if (base64_encode_alloc((const char *) sha1_key, 20, &accept_key) == 0) {
        return NULL;
    }

    return accept_key; /* need to free */
}


int ws_parse_request(ws_request_t *req, char *req_str)
{
    int status = 0;
    char *accept_key;
    char *save;
    char *token = strtok_r(req_str, "\r\n", &save);
    if (NULL == token) {
       status = _BAD_REQUEST;
       goto create_response;
    }

    if (strncasecmp(WS_REQUEST_GET_STR, token,
                WS_REQUEST_GET_LEN) != 0) {
        status = _BAD_REQUEST;
        goto create_response;
    }

    if (_parse_get(token, &req->uri, &req->http_version) != 0) {
        status = _BAD_REQUEST;
        goto create_response;
    }

    token = strtok_r(NULL, "\r\n", &save);
    while (token) {
        if (strncasecmp(WS_REQUEST_HOST_STR, token,
                    WS_REQUEST_HOST_LEN) == 0) {
            req->host = token + WS_REQUEST_HOST_LEN;
        } else if (strncasecmp(WS_REQUEST_UPGRADE_STR, token,
                    WS_REQUEST_UPGRADE_LEN) == 0) {
            req->upgrade = token + WS_REQUEST_UPGRADE_LEN;
        } else if (strncasecmp(WS_REQUEST_CONNECTION_STR, token,
                    WS_REQUEST_CONNECTION_LEN) == 0) {
            req->connection= token + WS_REQUEST_CONNECTION_LEN;
        } else if (strncasecmp(WS_REQUEST_KEY_STR, token,
                    WS_REQUEST_KEY_LEN) == 0) {
            req->key= token + WS_REQUEST_KEY_LEN;
        } else if (strncasecmp(WS_REQUEST_ORIGIN_STR, token,
                    WS_REQUEST_ORIGIN_LEN) == 0) {
            req->origin = token + WS_REQUEST_ORIGIN_LEN;
        } else if (strncasecmp(WS_REQUEST_PROTOCOL_STR, token,
                    WS_REQUEST_PROTOCOL_LEN) == 0) {
            req->protocol_str = token + WS_REQUEST_PROTOCOL_LEN;
        } else if (strncasecmp(WS_REQUEST_VERSION_STR, token,
                    WS_REQUEST_VERSION_LEN) == 0) {
            req->version_str = token + WS_REQUEST_VERSION_LEN;
        }
        token = strtok_r(NULL, "\r\n", &save);
    }

    /* check http version */
    if (NULL == req->http_version ||
            strcasecmp(req->http_version, _HTTP_VERSION_STR)) {
        status = _BAD_REQUEST;
        goto create_response;
    }

    /* check upgrade */
    if (NULL == req->upgrade || strcasecmp(req->upgrade, _UPGRADE_STR)) {
        status = _BAD_REQUEST;
        goto create_response;
    }

    /* check host */
    if (NULL == req->host) {
        status =  _BAD_REQUEST;
        goto create_response;
    }

    /* check origin */
    if (NULL == req->origin) {
       /* this shakehand from a client */
        ;
    }

    /* check protocol */
    if (NULL != req->protocol_str) {
        token = strtok_r(req->protocol_str, ", ", &save);
        while (token) {
            unsigned i;
            for (i = 1; i < sizeof(_accept_protocol); ++i) {
                if (!strcasecmp(token, _accept_protocol[i])) {
                    req->protocol = i;
                    break;
                }
            }
            if (req->protocol != WS_PROTOCOL_UNKNOW) {
               break;
            }
            token = strtok_r(NULL, ", ", &save);
        }

        if (req->protocol == WS_PROTOCOL_UNKNOW) {
            return  _NOT_IMPLEMENTED;
        }
    }

    /* check websocket version */
    if (NULL == req->version_str) {
        status =  _BAD_REQUEST;
        goto create_response;
    }
    if (strcasecmp(req->version_str, "13")) {
        status = _UPGRADE_REQUIRED;
        goto create_response;
    }

    if (NULL == req->key ||
            (accept_key = _create_accept_key(req->key)) == NULL) {
        status = _BAD_REQUEST;
        goto create_response;
    }

    req->version = 13;
    status = 0;

create_response:
    if (status != _VALID_REQUEST) {
        strncpy(req->response, _status_lines[status], sizeof(req->response));
        return -status;
    } else {
        int len = 0;
        len += snprintf(req->response + len, WS_REQUEST_LEN - len,
                "%s", _status_lines[status]);
        len += snprintf(req->response + len, WS_REQUEST_LEN - len,
                "Sec-WebSocket-Accept: %s\r\n", accept_key);
        free(accept_key);
        if (req->protocol != WS_PROTOCOL_UNKNOW) {
            len += snprintf(req->response + len, WS_REQUEST_LEN - len,
                    "%s%s\r\n", WS_REQUEST_PROTOCOL_STR,
                    _accept_protocol[req->protocol]);
        }

        len += snprintf(req->response + len, WS_REQUEST_LEN - len, "\r\n");
        return status;
    }
}

