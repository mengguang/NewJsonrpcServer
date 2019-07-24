//
// Created by mengguang on 7/24/19.
//

#include "newjsonrpc.h"

static char read_buffer[MAX_READ_BUFFER_SIZE];

static int send_response(jrpc_server *server, char *response) {
    if (server->debug_level > 0) {
        fprintf(stderr, "JSON Response: %s\n", response);
    }
    fprintf(stdout, "\x1e%s\n", response);
    fflush(stdout);
    return 0;
}

static int send_result(jrpc_server *server, cJSON *result, cJSON *id) {
    int return_value = 0;
    cJSON *result_root = cJSON_CreateObject();
    if (result) {
        cJSON_AddItemToObject(result_root, "result", result);
    }
    cJSON_AddItemToObject(result_root, "id", id);

    char *str_result = cJSON_PrintUnformatted(result_root);
    return_value = send_response(server, str_result);
    free(str_result);
    cJSON_Delete(result_root);
    return return_value;
}

static int send_error(jrpc_server *server, int code, char *message, cJSON *id) {
    int return_value = 0;
    cJSON *result_root = cJSON_CreateObject();
    cJSON *error_root = cJSON_CreateObject();
    cJSON_AddNumberToObject(error_root, "code", code);
    cJSON_AddStringToObject(error_root, "message", message);
    cJSON_AddItemToObject(result_root, "error", error_root);
    cJSON_AddItemToObject(result_root, "id", id);
    char *str_result = cJSON_Print(result_root);
    return_value = send_response(server, str_result);
    free(str_result);
    cJSON_Delete(result_root);
    free(message);
    return return_value;
}

static int invoke_procedure(jrpc_server *server, char *name, cJSON *params, cJSON *id) {
    cJSON *returned = NULL;
    int procedure_found = 0;
    jrpc_context ctx;
    ctx.error_code = 0;
    ctx.error_message = NULL;
    int i = server->procedure_count;
    while (i--) {
        if (!strcmp(server->procedures[i].name, name)) {
            procedure_found = 1;
            ctx.data = server->procedures[i].data;
            returned = server->procedures[i].function(&ctx, params, id);
            break;
        }
    }
    if (!procedure_found)
        return send_error(server, JRPC_METHOD_NOT_FOUND,
                          strdup("Method not found."), id);
    else {
        if (ctx.error_code)
            return send_error(server, ctx.error_code, ctx.error_message, id);
        else
            return send_result(server, returned, id);
    }
}

static int eval_request(jrpc_server *server, cJSON *root) {
    cJSON *method, *params, *id;
    method = cJSON_GetObjectItem(root, "method");
    if (method != NULL && method->type == cJSON_String) {
        params = cJSON_GetObjectItem(root, "params");
        if (params == NULL || params->type == cJSON_Array
            || params->type == cJSON_Object) {
            id = cJSON_GetObjectItem(root, "id");
            if (id == NULL || id->type == cJSON_String
                || id->type == cJSON_Number) {
                //We have to copy ID because using it on the reply and deleting the response Object will also delete ID
                cJSON *id_copy = NULL;
                if (id != NULL) {
                    id_copy =
                            (id->type == cJSON_String) ? cJSON_CreateString(
                                    id->valuestring) :
                            cJSON_CreateNumber(id->valueint);
                }
                if (server->debug_level > 0) {
                    fprintf(stderr, "Method Invoked: %s\n", method->valuestring);
                }
                return invoke_procedure(server, method->valuestring,
                                        params, id_copy);
            }
        }
    }
    send_error(server, JRPC_INVALID_REQUEST,
               strdup("The JSON sent is not a valid Request object."), NULL);
    return -1;
}

int jrpc_register_procedure(jrpc_server *server,
                            jrpc_function function_pointer, char *name, void *data) {
    int i = server->procedure_count++;
    if (!server->procedures) {
        server->procedures = malloc(sizeof(struct jrpc_procedure));
    } else {
        struct jrpc_procedure *ptr = realloc(server->procedures,
                                             sizeof(struct jrpc_procedure) * server->procedure_count);
        if (!ptr)
            return -1;
        server->procedures = ptr;

    }
    if ((server->procedures[i].name = strdup(name)) == NULL)
        return -1;
    server->procedures[i].function = function_pointer;
    server->procedures[i].data = data;
    return 0;
}

int jrpc_server_init(jrpc_server *server, int debug_level) {
    memset(server, 0, sizeof(jrpc_server));
    server->debug_level = debug_level;
}

void jrpc_server_loop(jrpc_server *server) {
    while (true) {
        memset(read_buffer, 0, sizeof(read_buffer));
        char *result = fgets(read_buffer, sizeof(read_buffer), stdin);
        if (result == NULL) {
            if (server->debug_level > 0) {
                fprintf(stderr, "Error: fgets error, abort.\n");
            }
            return;
        }
        if (result[0] != 0x1E) {
            if (server->debug_level > 0) {
                fprintf(stderr, "Error: data is not 0x1E prefixed.\n");
            }
            continue;
        }
        size_t length = strlen(result);
        if (result[length - 1] != '\n') {
            if (server->debug_level > 0) {
                fprintf(stderr, "Error: data is not \\n ended or longer than %lu bytes.\n", sizeof(read_buffer) - 1);
            }
            continue;
        }
        if (server->debug_level > 0) {
            fprintf(stderr, "Raw data: %s", result + 1);
        }
        cJSON *root = cJSON_Parse(result + 1);
        if (root == NULL) {
            if (server->debug_level > 0) {
                fprintf(stderr, "cJSON_Parse error.\n");
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL) {
                    fprintf(stderr, "Error before: %s\n", error_ptr);
                }
            }
            cJSON_Delete(root);
            continue;
        } else {
            if (server->debug_level > 0) {
                char *str_result = cJSON_PrintUnformatted(root);
                fprintf(stderr, "Valid JSON Received: %s\n", str_result);
                free(str_result);
            }
            if (root->type == cJSON_Object) {
                eval_request(server, root);
            }
            cJSON_Delete(root);
        }
    }
}
