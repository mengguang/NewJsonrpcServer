//
// Created by mengguang on 7/24/19.
//

#ifndef NEWJSONRPCSERVER_NEWJSONRPC_H
#define NEWJSONRPCSERVER_NEWJSONRPC_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cJSON.h"

#define MAX_READ_BUFFER_SIZE 64

/*
 *
 * https://tools.ietf.org/html/rfc7464
 * A JSON text sequence consists of any number of JSON texts, all encoded
 * in UTF-8, each prefixed by an ASCII Record Separator (0x1E), and each
 * ending with an ASCII Line Feed character (0x0A).
 */

/*
 *
 * http://www.jsonrpc.org/specification
 *
 * code	message	meaning
 * -32700	Parse error	Invalid JSON was received by the server.
 * An error occurred on the server while parsing the JSON text.
 * -32600	Invalid Request	The JSON sent is not a valid Request object.
 * -32601	Method not found	The method does not exist / is not available.
 * -32602	Invalid params	Invalid method parameter(s).
 * -32603	Internal error	Internal JSON-RPC error.
 * -32000 to -32099	Server error	Reserved for implementation-defined server-errors.
 */

#define JRPC_PARSE_ERROR -32700
#define JRPC_INVALID_REQUEST -32600
#define JRPC_METHOD_NOT_FOUND -32601
#define JRPC_INVALID_PARAMS -32603
#define JRPC_INTERNAL_ERROR -32693

typedef struct {
    void *data;
    int error_code;
    char *error_message;
} jrpc_context;

typedef cJSON *(*jrpc_function)(jrpc_context *context, cJSON *params, cJSON *id);

struct jrpc_procedure {
    char *name;
    jrpc_function function;
    void *data;
};

typedef struct {
    int procedure_count;
    struct jrpc_procedure *procedures;
    int debug_level;
} jrpc_server;


int jrpc_register_procedure(jrpc_server *server, jrpc_function function_pointer, char *name, void *data);

int jrpc_server_init(jrpc_server *server, int debug_level);

void jrpc_server_loop(jrpc_server *server);

#endif //NEWJSONRPCSERVER_NEWJSONRPC_H
