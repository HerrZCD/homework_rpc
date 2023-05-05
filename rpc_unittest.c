#include "rpc.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

rpc_data *add2_i8(rpc_data *data) {
    return malloc(sizeof(rpc_data));
}

struct rpc_handle {
    long method_address;
};

struct rpc_server {
    /* Add variable(s) for server state */
    int server_fd;
    struct sockaddr* socket_address;
    socklen_t* address_len;
    method* methods[MAX_METHOD_NUMBER]; 
};

void rpc_register_normal() {
    rpc_server* server = (rpc_server*)malloc(sizeof(rpc_server));
    memset(server, 0, sizeof(rpc_server));
    server->server_fd = 1;
    method* m = (method*)malloc(sizeof(method));
    m->name = "TestName1";
    m->handler = add2_i8;
    server->methods[0] = m;

    int ret_val = rpc_register(server, "TestName2", add2_i8);
    assert(ret_val == 1);
    assert(strcmp(server->methods[0]->name, "TestName1") == 0);
    assert(strcmp(server->methods[1]->name, "TestName2") == 0);
}

void rpc_register_duplicate() {
    rpc_server* server = (rpc_server*)malloc(sizeof(rpc_server));
    memset(server, 0, sizeof(rpc_server));
    server->server_fd = 1;
    method* m = (method*)malloc(sizeof(method));
    memset(m, 0, sizeof(method));
    m->name = "TestName1";
    m->handler = add2_i8;
    server->methods[0] = m;

    int ret_val = rpc_register(server, "TestName1", add2_i8);
    assert(ret_val == 1);
    assert(strcmp(server->methods[0]->name, "TestName1") == 0);
    assert(server->methods[1] == NULL);
}

void get_rpc_handle_normal() {
    rpc_server* server = (rpc_server*)malloc(sizeof(rpc_server));
    memset(server, 0, sizeof(rpc_server));
    server->server_fd = 1;
    rpc_register(server, "TestName1", add2_i8);
    rpc_handle* handle = get_rpc_handle(server, "TestName1");
    assert(handle->method_address == (long)add2_i8);
}

int main() {
  rpc_register_normal();
  rpc_register_duplicate();
  get_rpc_handle_normal();
  return 0;
}