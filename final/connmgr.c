#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/stat.h> 

#include "config.h"
#include "connmgr.h"
#include "lib/tcpsock.h"
#include "lib/dplist.h" 

#define CONNMGR_GET_DATA_FAILURE 0
#define CONNMGR_GET_DATA_SUCCESS 1

#define CONNMGR_INVALID_ERROR   2
#define CONNMGR_MEMORY_ERROR    3
#define CONNMGR_SOCKET_ERROR    4
#define CONNMGR_FD_ERROR        5

#define REMOVE_FD -1                // value to be assigned to disconnected fds so they can be removed from "*fds"
#define TIMEOUT_MULTIPLIER 1000     // value that will be multiplied with TIMEOUT, currently seconds to mseconds

typedef struct {
    tcpsock_t *socket;
    time_t timestamp;
    sensor_data_t sensor_data;
}connection;

struct pollfd *fds;
dplist_t *connmgr_list;

//callback functions for connmgr_list
connection *copy_connection(connection*);
void **free_connection(connection **);
int compare_connection(connection*, connection*);

// inititializes dplist and establishes connection with server
void init_server(int);

// if there is a new client, establish connection with the new client
void insert_client(connection*, int, void(*write_to_buffer)(sensor_data_t*), void(*write_to_fifo)(char*));

// if no response from client or server, remove connection 
void close_connection(connection*, int, void(*write_to_fifo)(char*));

// gets the id, temp, and ts of a specific client, returnrs if operation was a success or failure 
char read_data(connection *client);

// the data read from the client is sent to the buffer
void read_data_write_to_buffer(connection *client, int index, void(*write_to_buffer)(sensor_data_t*), void(*write_to_fifo)(char*));

// gets rid of fds that get disconnected
void compress(int, int);

void connmgr_listen_buffer(int port_number, void(*write_to_buffer)(sensor_data_t*), void(*write_to_fifo)(char*)) {
    
    init_server(port_number);
    int size, index;
    connection *conn;

    while(1) {
        size = dpl_size(connmgr_list);

        if(poll(fds, size, TIMEOUT*TIMEOUT_MULTIPLIER) <= 0) break;   // poll call failed or timedout

        for(index = 0; index < size; index++) {

            if((fds+index)->revents == 0) continue; // skip if no returned events

            conn = dpl_get_element_at_index(connmgr_list, index);

            if((fds+index)->revents == POLLRDHUP || (fds+index)->revents != POLLIN) {   // if closed or not pollin, close connection
                close_connection(conn, index, write_to_fifo);
                continue;
            }   
            if(index == 0) insert_client(conn, size, write_to_buffer, write_to_fifo);      // server poll
            else read_data_write_to_buffer(conn, index, write_to_buffer, write_to_fifo);   // client poll
        }
        compress(size, dpl_size(connmgr_list)); // remove disconnected fds 
    }
    conn = dpl_get_element_at_index(connmgr_list, 0);   // closing server
    close_connection(conn, 0, write_to_fifo);
    free(fds);
}

void connmgr_free(void) {
    dpl_free(&connmgr_list, true);
}

//private functions

void init_server(int port_number) {
    connmgr_list = dpl_create(  (void *(*)(void *))copy_connection, (void (*)(void **))free_connection, (int (*)(void *, void *))compare_connection );

    connection *server = (connection*)malloc(sizeof(connection));
    ERR_HANDLER(server == NULL, CONNMGR_MEMORY_ERROR);

    int server_fd, error_code;
    error_code = tcp_passive_open(&(server->socket), port_number);
    ERR_HANDLER(error_code != TCP_NO_ERROR, CONNMGR_SOCKET_ERROR);
    error_code = tcp_get_sd(server->socket, &server_fd); 
    ERR_HANDLER(error_code != TCP_NO_ERROR, CONNMGR_SOCKET_ERROR);

    DEBUG_PRINTF("server fd:(%d) started\n", server_fd);

    fds = (struct pollfd*)malloc(sizeof(struct pollfd));
    ERR_HANDLER(fds == NULL, CONNMGR_MEMORY_ERROR);

    memset(fds, 0, sizeof(struct pollfd));
    fds->fd = server_fd;
    fds->events = POLLIN;

    server->timestamp = time(NULL);
    server->sensor_data.id = 0;
    server->sensor_data.value = 0;
    server->sensor_data.ts = 0;
    dpl_insert_at_index(connmgr_list, server, 0, false);
}

void insert_client(connection *server, int index, void(*write_to_buffer)(sensor_data_t*), void(*write_to_fifo)(char*)) {
    ERR_HANDLER(server == NULL, CONNMGR_INVALID_ERROR);
    ERR_HANDLER(index <= 0, CONNMGR_INVALID_ERROR);
    ERR_HANDLER(connmgr_list == NULL, CONNMGR_INVALID_ERROR);

    connection *client = (connection*)malloc(sizeof(connection));
    ERR_HANDLER(client == NULL, CONNMGR_MEMORY_ERROR);

    int client_fd, error_code;
    error_code = tcp_wait_for_connection(server->socket, &(client->socket));
    ERR_HANDLER(error_code != TCP_NO_ERROR, CONNMGR_SOCKET_ERROR);
    error_code = tcp_get_sd(client->socket, &client_fd);
    ERR_HANDLER(error_code != TCP_NO_ERROR, CONNMGR_SOCKET_ERROR);

    fds = (struct pollfd*)realloc(fds, (index+1)*sizeof(struct pollfd));
    ERR_HANDLER(fds == NULL, CONNMGR_MEMORY_ERROR);
    
    memset((fds+index), 0, sizeof(struct pollfd));
    (fds+index)->fd = client_fd;
    (fds+index)->events = POLLIN | POLLRDHUP;
    client->timestamp = time(NULL);
    server->timestamp = time(NULL);

    DEBUG_PRINTF("client fd:(%d) started\n", client_fd);

    char fifo_buffer[CONFIG_FIFO_SIZE];
    if(read_data(client) == CONNMGR_GET_DATA_SUCCESS) {
        dpl_insert_at_index(connmgr_list, client, index, false);

        snprintf(fifo_buffer, CONFIG_FIFO_SIZE, "A sensor node with id:%d has opened a new connection\n", client->sensor_data.id);
        write_to_fifo(fifo_buffer);

        write_to_buffer(&client->sensor_data);
    }
}

void close_connection(connection *conn, int index, void(*write_to_fifo)(char*)) {
    ERR_HANDLER(conn == NULL, CONNMGR_INVALID_ERROR);
    ERR_HANDLER(index < 0, CONNMGR_INVALID_ERROR);
    ERR_HANDLER(connmgr_list == NULL, CONNMGR_INVALID_ERROR);

    int error_code;
    error_code = tcp_close(&(conn->socket));
    ERR_HANDLER(error_code != TCP_NO_ERROR, CONNMGR_SOCKET_ERROR);

    connmgr_list = dpl_remove_at_index(connmgr_list, index, true);
    char fifo_buffer[CONFIG_FIFO_SIZE];
    if(index > 0) {
        snprintf(fifo_buffer, CONFIG_FIFO_SIZE, "The sensor node with id:%d has closed the conection\n", conn->sensor_data.id);
        write_to_fifo(fifo_buffer);
    }
    DEBUG_PRINTF("server/client fd:(%d) closed connection\n", (fds+index)->fd);
    (fds+index)->fd = REMOVE_FD;
}

char read_data(connection *client) {
    ERR_HANDLER(client == NULL, CONNMGR_INVALID_ERROR);
    int bytes, error_code;
    sensor_data_t data;

    // read sensor ID
    bytes = sizeof(data.id);
    error_code = tcp_receive(client->socket,(void *)&data.id,&bytes);
    ERR_HANDLER(error_code != TCP_NO_ERROR, CONNMGR_SOCKET_ERROR);
    // read temperature
    bytes = sizeof(data.value);
    error_code = tcp_receive(client->socket,(void *)&data.value,&bytes);
    ERR_HANDLER(error_code != TCP_NO_ERROR, CONNMGR_SOCKET_ERROR);
    // read timestamp
    bytes = sizeof(data.ts);
    error_code = tcp_receive( client->socket, (void *)&data.ts,&bytes);
    ERR_HANDLER(error_code != TCP_NO_ERROR, CONNMGR_SOCKET_ERROR);

    if(bytes > 0) {
        client->sensor_data.id = data.id;
        client->sensor_data.value = data.value;
        client->sensor_data.ts = data.ts;
        return CONNMGR_GET_DATA_SUCCESS;
    } 
    return CONNMGR_GET_DATA_FAILURE;
}

void read_data_write_to_buffer(connection *client, int index, void(*write_to_buffer)(sensor_data_t*), void(*write_to_fifo)(char*)) {
    ERR_HANDLER(client == NULL, CONNMGR_INVALID_ERROR);
    ERR_HANDLER(index <= 0, CONNMGR_INVALID_ERROR);

    if(read_data(client) == CONNMGR_GET_DATA_SUCCESS) {
        write_to_buffer(&client->sensor_data);
        client->timestamp = time(NULL);
    }    
    else close_connection(client, index, write_to_fifo);
}

void compress(int old_size, int new_size) {
    ERR_HANDLER(old_size < 0, CONNMGR_INVALID_ERROR);
    ERR_HANDLER(new_size < 0, CONNMGR_INVALID_ERROR);
    if(old_size == new_size) return;
    int i, j, k;

    for(i = 0; i < old_size; i++) {
        if((fds+i)->fd == REMOVE_FD) {
            for(j = i; j < old_size - 1; j++) {
                (fds+j)->fd = (fds+j+1)->fd;
                (fds+j+1)->fd = REMOVE_FD;
            }
        }
    }
    for(k = 0; k < new_size; k++) if((fds+k)->fd == REMOVE_FD) compress(old_size, new_size);
}

// callback functions

connection *copy_connection(connection *conn) {
    ERR_HANDLER(conn == NULL, CONNMGR_INVALID_ERROR);
    connection *new_conn = (connection*)malloc(sizeof(connection));
    ERR_HANDLER(new_conn == NULL, CONNMGR_MEMORY_ERROR);
    *new_conn = *conn;
    return new_conn;
}

void **free_connection(connection **conn) {
    ERR_HANDLER(*conn == NULL, CONNMGR_INVALID_ERROR);
    free(*conn);
    *conn = NULL;
    return (void**)conn;
}

int compare_connection(connection *conn1, connection *conn2) {
    ERR_HANDLER(conn1 == NULL, CONNMGR_INVALID_ERROR);
    ERR_HANDLER(conn2 == NULL, CONNMGR_INVALID_ERROR);
    int sd1, sd2, error_code;
    error_code = tcp_get_sd(conn1->socket, &sd1);
    ERR_HANDLER(error_code != TCP_NO_ERROR, CONNMGR_SOCKET_ERROR);
    error_code = tcp_get_sd(conn2->socket, &sd2);
    ERR_HANDLER(error_code != TCP_NO_ERROR, CONNMGR_SOCKET_ERROR);
    if(sd1 > sd2) return 1;
    if(sd1 < sd2) return -1;
    return 0;
}
