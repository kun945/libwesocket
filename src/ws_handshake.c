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

static uint32_t _seed = 1314520;

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


static char * _create_request_key()
{
    char bytes[16];
    srand(_seed);
    unsigned i;
    uint32_t sum = 0;
    for (i = 0; i < sizeof(bytes); ++i) {
        bytes[i] = rand() % 0x100;
        sum += bytes[i];
    }
    _seed = (_seed + sum) * 1102515245;
    char *request_key;
    if (base64_encode_alloc(bytes, 16, &request_key) == 0) {
        return NULL;
    }

    return request_key; /* need to be free */
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

    return accept_key; /* need to be free */
}


int ws_parse_request(ws_handshake_ctx_t *req, char *req_str)
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
        strncpy(req->send_str, _status_lines[status], sizeof(req->send_str));
        return status;
    } else {
        int len = 0;
        len += snprintf(req->send_str + len, WS_REQUEST_LEN - len,
                "%s", _status_lines[status]);
        len += snprintf(req->send_str + len, WS_REQUEST_LEN - len,
                "Sec-WebSocket-Accept: %s\r\n", accept_key);
        free(accept_key);
        if (req->protocol != WS_PROTOCOL_UNKNOW) {
            len += snprintf(req->send_str + len, WS_REQUEST_LEN - len,
                    "%s%s\r\n", WS_REQUEST_PROTOCOL_STR,
                    _accept_protocol[req->protocol]);
        }

        len += snprintf(req->send_str + len, WS_REQUEST_LEN - len, "\r\n");
        return status;
    }
}


int ws_create_request(ws_request_ctx_t *req, const char *uri)
{
    if (strncasecmp(WS_URI_PREFIX_STR, uri, WS_URI_PREFIX_LEN)) {
        return -1;
    }

    int i= WS_URI_PREFIX_LEN;
    unsigned host_len;
    char host[32];
    char port[16];
    char path[64];
    char *path_s = index(uri + i, '/');
    char *port_s = index(uri + i, ':');
    if (NULL == port_s && NULL == path_s) {
        snprintf(port, sizeof(port), "%s", "80");
        snprintf(path, sizeof(path), "%s", "/");
        host_len = strlen(uri + i);
    } else if (NULL == port_s && NULL != path_s) {
        snprintf(port, sizeof(port), "%s", "80");
        snprintf(path, sizeof(path), "%s", path_s);
        host_len = path_s - (uri + i);
    } else if (NULL != port_s && NULL == path_s) {
        snprintf(port, sizeof(port), "%s", port_s + 1); /* skip ':' */
        snprintf(path, sizeof(path), "%s", "/");
        host_len = port_s - (uri + i);
    } else if (NULL != port_s && NULL != path_s) {
        snprintf(path, sizeof(path), "%s", path_s);
        unsigned port_len = path_s - (port_s + 1);
        if (port_len >= sizeof(port)) {
            return -1;
        }
        host_len = port_s - (uri + i);
        memcpy(port, port_s + 1, port_len); /* skip ':' */
        port[port_len] = 0;
    }


    if (host_len >= sizeof(host)) {
        return -1;
    }
    memcpy(host, uri + i, host_len);
    host[host_len] = 0;

    char *key = _create_request_key();
    if (NULL == key) {
        return -1;
    }

    snprintf(req->send_str, sizeof(req->send_str), "GET %s HTTP/1.1\r\n"
            "Host: %s\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
            "Sec-WebSocket-Key: %s\r\nSec-WebSocket-Protocol: chat\r\n"
            "Sec-WebSocket-Version: 13\r\n\r\n", path, host, key);

    char *accept_key = _create_accept_key(key);
    if (NULL == accept_key) {
        free(key);
        return -1;
    }

    printf("accept_key %s\n", accept_key);
    snprintf(req->host, sizeof(req->host), "%s", host);
    snprintf(req->request_key, sizeof(req->request_key), "%s", key);
    snprintf(req->accept_key, sizeof(req->accept_key), "%s", accept_key);
    req->port = atoi(port);

    free(accept_key);
    free(key);
    return 0;
}



static int _parse_status_code(char *status_line)
{
    char *code_str = status_line + WS_RESPONSE_VERSION_LEN;
    int i = 0;
    while (code_str[i]) {
        if (code_str[i] == ' ') {
            code_str[i]  = 0;
            break;
        }
        ++i;
    }

    return atoi(code_str); /* return status code */
}


int ws_parse_response(ws_request_ctx_t *req, char *resp_str)
{
    char *save;
    char *token = strtok_r(resp_str, "\r\n", &save);
    if (NULL == token) {
        return -1;
    }

    if (strncasecmp(token, WS_RESPONSE_VERSION_STR,
                WS_RESPONSE_VERSION_LEN) != 0) {
        return -1;
    }

    int status_code = _parse_status_code(token);
    req->status_code = status_code;
    if (status_code != 101) {
        return -1;
    }

    char *version = NULL;
    char *upgrade = NULL;
    char *connection = NULL;
    char *accept_key = NULL;
    char *protocol = NULL;
    token = strtok_r(NULL, "\r\n", &save);
    while (token) {
        if (strncasecmp(WS_RESPONSE_UPGRADE_STR, token,
                    WS_RESPONSE_UPGRADE_LEN) == 0) {
            upgrade = token + WS_REQUEST_UPGRADE_LEN;
        } else if (strncasecmp(WS_RESPONSE_CONNECTION_STR, token,
                    WS_RESPONSE_CONNECTION_LEN) == 0) {
            connection= token + WS_REQUEST_CONNECTION_LEN;
        } else if (strncasecmp(WS_RESPONSE_ACCEPT_STR, token,
                    WS_RESPONSE_ACCEPT_LEN) == 0) {
            accept_key = token + WS_REQUEST_KEY_LEN;
        } else if (strncasecmp(WS_REQUEST_PROTOCOL_STR, token,
                    WS_REQUEST_PROTOCOL_LEN) == 0) {
            protocol= token + WS_REQUEST_PROTOCOL_LEN;
        } else if (strncasecmp(WS_REQUEST_VERSION_STR, token,
                    WS_REQUEST_VERSION_LEN) == 0) {
            version= token + WS_REQUEST_VERSION_LEN;
        }
        token = strtok_r(NULL, "\r\n", &save);
    }

    /* check http version */
    if (NULL == version ||
            strcasecmp(version, _HTTP_VERSION_STR)) {
        return -1;
    }

    /* check upgrade */
    if (NULL == upgrade || strcasecmp(upgrade, _UPGRADE_STR)) {
        return -1;
    }

    /* check protocol */
    if (NULL != protocol) {
        return -1;
    }

    /* check websocket version */
    if (NULL == version) {
        return -1;
    }

    if (strcasecmp(version, "13")) {
        return -1;
    }

    if (strncmp(req->accept_key, accept_key, strlen(req->accept_key))) {
        return -1;
    }

    return 0;
}
