#include "rpc.h"
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

struct rpc_client {
    int status;
    int client_fd;
    struct sockaddr_in serv_addr;
};

struct rpc_handle {
    long method_address;
};

struct rpc_server {
    /* Add variable(s) for server state */
    int server_fd;
    int socket;
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
    rpc_server* server = (rpc_server*)malloc(sizeof(rpc_server));
    memset(server, 0, sizeof(rpc_server));

    int address_len = sizeof(address);

    server->address_len = (socklen_t*)&address_len;
    server->socket_address = (struct sockaddr*)&address;
    server->server_fd = server_fd;
    server->socket = new_socket;
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
    char buffer[1024] = { 0 };
	char* hello = "Hello from server";
    while (1) {
        if (listen(srv->server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        if ((srv->socket
            = accept(srv->server_fd, srv->socket_address,
                    srv->address_len))
            < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        int valread = recv(srv->socket, buffer, 1024, 0);
        request* readed_data = (request*)buffer;
        printf("readed %s\n", (const char*)readed_data);
        rpc_handle* handle = NULL;
        if (readed_data && (readed_data->request_method == 1)) {
            printf("handle rpc find\n");
            printf("the requested functions is %s\n", (const char*)&readed_data->data.data2);
            handle = get_rpc_handle(srv, (const char*)&readed_data->data.data2);
            if (handle != NULL) {
                printf("handle address is %ld\n", handle->method_address);
                send(srv->socket, (void*)handle, sizeof(rpc_handle), 0);
                free(handle);
            } else {
                char* error = "No handle is found";
                send(srv->socket, error, strlen(error), 0);
            }
        } else if (readed_data->request_method == 2) {
            printf("handle rpc find\n");
            printf("execute2\n");
            printf("the requested functions is %d\n", (int)*(char*)&readed_data->data.data2);
            printf("execute1\n");
            printf("value1 is %d\n", readed_data->data.data1);
            printf("length is %d\n", (int)readed_data->data.data2_len);
            printf("addr is %ld\n", readed_data->address);
            // send(srv->socket, hello, strlen(hello), 0);
            rpc_data data;
            data.data1 = readed_data->data.data1;
            data.data2 = (char*)&readed_data->data.data2;
            data.data2_len = 1;

            rpc_data* result = ((rpc_handler)readed_data->address)(&data);
            printf("result is %d\n", result->data1);

            rpc_data* response = (rpc_data*)malloc(sizeof(rpc_data));
            memset(response, 0, sizeof(rpc_data));
            if (result == NULL) {
                // Error handling of add2.
                send(srv->socket, (void*)response, sizeof(rpc_data), 0);

            } else {
                response->data1 = result->data1;
                response->data2_len = result->data2_len;
                memcpy(&response->data2, &result->data1, 4);
                send(srv->socket, (void*)response, sizeof(rpc_data) + 5, 0);
                printf("sended result is %d\n", response->data1);

            }
            free(response);
        }
        else {
            printf("return hello\n");
            send(srv->socket, hello, strlen(hello), 0);
            printf("Hello message sent\n");
        }
    }

}

rpc_handle* get_rpc_handle(rpc_server *srv, const char* name) {
    if (srv == NULL) {
        return NULL;
    }
    for (int i = 0; i < MAX_METHOD_NUMBER; ++i) {
        if (srv->methods[i] == NULL) {
            return NULL;
        }
        if ((srv->methods[i] != NULL) && (srv->methods[i]->name != NULL) && (strlen(srv->methods[i]->name) != 0) && (strcmp(srv->methods[i]->name, name) == 0)) {
            rpc_handle* handle = (rpc_handle*)malloc(sizeof(rpc_handle));
            memset(handle, 0, sizeof(rpc_handle));
            printf("found addr, %ld\n", (long)srv->methods[i]->handler);
            handle->method_address = (long)srv->methods[i]->handler;
            return handle;
        }
    }
    return NULL;
}

rpc_client *rpc_init_client(char *addr, int port) {
    int status, valread, client_fd;
	struct sockaddr_in serv_addr;
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return NULL;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
    // TODO: Use IP adress from parameter.
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return NULL;
	}

	if ((status
		= connect(client_fd, (struct sockaddr*)&serv_addr,
				sizeof(serv_addr)))
		< 0) {
		printf("\nConnection Failed \n");
		return NULL;
	}
    rpc_client* client = (rpc_client*)malloc(sizeof(rpc_client));
    memset(client, 0, sizeof(rpc_client));
    client->status = status;
    client->client_fd = client_fd;
    client->serv_addr = serv_addr;
    return client;
}

rpc_handle *rpc_find(rpc_client *cl, char *name) {
    int size = sizeof(request) + strlen(name) + 1;
    request* data = (request*)malloc(size);
    memset(data, 0, size);
    memcpy(&data->data.data2, name, strlen(name) + 1);
    data->data.data2_len = strlen(name);
    data->request_method = 1;

    rpc_handle* handle = (rpc_handle*)malloc(sizeof(rpc_handle));
    memset(handle, 0, sizeof(rpc_handle));
    send(cl->client_fd, (void*)data, size, 0);
    printf("send items %s\n", (const char*)&data->data.data2);
	int valread = read(cl->client_fd, (void*)handle, 1024);
    printf("received func addr %ld\n", handle->method_address);
    free(data);
    close(cl->client_fd);
    return handle;
}

rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    if ((cl->client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return NULL;
	}
    int status;
    if ((status
		= connect(cl->client_fd, (struct sockaddr*)&cl->serv_addr,
				sizeof(cl->serv_addr)))
		< 0) {
		printf("\nConnection Failed \n");
		return NULL;
	}

    int size = sizeof(request) + 1 + 1;
    request* data = (request*)malloc(size);
    memset(data, 0, size);
    memcpy(&data->data.data2, payload->data2, 1);
    data->data.data2_len = 1;
    data->request_method = 2;
    data->data.data1 = payload->data1;
    data->address = h->method_address;

    send(cl->client_fd, (void*)data, size, 0);
    printf("send items %s\n", (const char*)&data->data.data2);
    char* buffer = (char*)malloc(sizeof(rpc_data)+5);
    memset(buffer, 0, sizeof(rpc_data));

	int valread = read(cl->client_fd, (void*)buffer, sizeof(rpc_data)+5);
    printf("buffer is %s", buffer);
    rpc_data* response = (rpc_data*)buffer;
    response->data2 = NULL;
    response->data2_len = 0;
    printf("received func result %d\n", response->data1);
    free(data);
    close(cl->client_fd);
    return response;
}

void rpc_close_client(rpc_client *cl) {
  free(cl);
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
