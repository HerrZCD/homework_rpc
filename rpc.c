#include "rpc.h"
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct rpc_server {
    /* Add variable(s) for server state */
    int server_fd;
    struct sockaddr* socket_address;
    socklen_t* address_len;
    method* methods[MAX_METHOD_NUMBER]; 
};

rpc_server *rpc_init_server(int port) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        return NULL;
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                SO_REUSEADDR, &opt,
                sizeof(opt))) {
        perror("setsockopt");
        return NULL;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
            sizeof(address))
        < 0) {
        perror("bind failed");
        return NULL;
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return NULL;
    }
    rpc_server* server = (rpc_server*)malloc(sizeof(rpc_server));
    memset(server, 0, sizeof(rpc_server));

    server->address_len = (socklen_t*)sizeof(address);
    server->socket_address = (struct sockaddr*)&address;
    server->server_fd = server_fd;
    return server;

}

int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {
    method** registed_methods = srv->methods;
    int index = 0;
    for (int i = 0; i < MAX_METHOD_NUMBER; ++i) {
        if ((registed_methods[i] != NULL) && (registed_methods[i]->name != NULL) && (strlen(registed_methods[i]->name) != 0) && (strcmp(registed_methods[i]->name, name) == 0)) {
            registed_methods[i]->handler = handler;
            break;
        }

        if ((registed_methods[i] != NULL) && (registed_methods[i]->name != NULL) && (strlen(registed_methods[i]->name) != 0)) {
          ++ index;
        } else {
            break;
        }
    }
    if (index == MAX_METHOD_NUMBER) {
        // Error: This methods has reach max numbers.
        return -1;
    } else {
        method* m = (method*)malloc(sizeof(method));
        memset(m, 0, sizeof(method));
        m->name = name;
        m->handler = handler;
        registed_methods[index] = m;
        return 1;
    }
    return -1;
}

void rpc_serve_all(rpc_server *srv) {

}

struct rpc_client {
    /* Add variable(s) for client state */
};

struct rpc_handle {
    /* Add variable(s) for handle */
};

rpc_client *rpc_init_client(char *addr, int port) {
    return NULL;
}

rpc_handle *rpc_find(rpc_client *cl, char *name) {
    return NULL;
}

rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    return NULL;
}

void rpc_close_client(rpc_client *cl) {

}

void rpc_data_free(rpc_data *data) {
    if (data == NULL) {
        return;
    }
    if (data->data2 != NULL) {
        free(data->data2);
    }
    free(data);
}
