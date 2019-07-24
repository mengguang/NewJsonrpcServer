#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "newjsonrpc.h"

/*
 *
 * test case:
 * echo -n -e "\x1e hello,world\x0a"|./NewJsonrpcServer
 * echo -n -e "\x1e hello,world hello world hello world hello world hello world hello world\x0a\x1e hello\x0a"|./NewJsonrpcServer
 * echo -n -e "\x1e{\"method\":\"sayHello\"}\n"|./NewJsonrpcServer
 *
 * network server with socat:
 * /usr/bin/socat tcp4-listen:16888,reuseaddr,fork exec:./NewJsonrpcServer
 *
 * network client with nc:
 * echo -n -e "\x1e{\"method\":\"sayHello\"}\n"|nc localhost 16888
 */


cJSON *say_hello(jrpc_context *ctx, cJSON *params, cJSON *id) {
    static uint32_t n = 0;
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Hello, %u\n", n++);
    return cJSON_CreateString(buffer);
}

int main() {
    jrpc_server server;
    jrpc_server_init(&server, 1);
    jrpc_register_procedure(&server, say_hello, "sayHello", NULL);
    jrpc_server_loop(&server);
}