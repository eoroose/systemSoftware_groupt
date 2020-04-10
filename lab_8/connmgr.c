#include "connmgr.h"

#define MAX_CONN 100

typedef struct {
    tcpsock_t *socket;
    time_t timestamp;
}connection;

struct pollfd fds[MAX_CONN+1];
dplist_t *list;
FILE *fp;

connection *copy_connection(connection*);
void **free_connection(connection **);
int compare_connection(connection*, connection*);

void init_server(int);
void insert_client(connection*, int);
void read_data(connection*, int);
void close_connection(connection*, int);
void compress(int);
void write_data_to_file(sensor_id_t, sensor_value_t, sensor_ts_t);

void connmgr_listen(int port_number) {
    fp = fopen("sensor_data_recv", "w");
    init_server(port_number);
    int size, index;
    connection *conn;

    do {
        //ERROR_HANDLER(fp == NULL,"Couldn't create sensor_data\n");  

        size = dpl_size(list);
        printf("Waiting on poll()...\n");

        if(poll(fds, size, TIMEOUT*1000) <= 0) break; //poll call failed or timedout

        for(index = 0; index < size; index++) {

            if(fds[index].revents == 0) continue;   //skip if no returned events

            conn = dpl_get_element_at_index(list, index);

            if(fds[index].revents == POLLRDHUP || fds[index].revents != POLLIN) { //if closed or not pollin, close connection
                close_connection(conn, index);
                continue;
            }   
            if(index == 0) insert_client(conn, size);       //server poll
            else read_data(conn, index);                            //client poll
        }
        compress(size); //gets rid of fds that became disconnected

    } while(size <= MAX_CONN);

    conn = dpl_get_element_at_index(list, 0);   //closing server
    close_connection(conn, 0);
    fclose(fp);
}

void connmgr_free(void) {
    dpl_free(&list, true);
}


connection *copy_connection(connection *conn) {
    connection *new_conn = (connection*)malloc(sizeof(connection));
    *new_conn = *conn;
    return new_conn;
}

void **free_connection(connection **conn) {
    free(*conn);
    *conn = NULL;
    return (void**)conn;
}

int compare_connection(connection *conn1, connection *conn2) {
    int sd1, sd2;
    tcp_get_sd(conn1->socket, &sd1);
    tcp_get_sd(conn2->socket, &sd2);
    if(sd1 > sd2) return 1;
    if(sd1 < sd2) return -1;
    return 0;
}

void init_server(int port_number) {
    list = dpl_create(  (void *(*)(void *))copy_connection,
                        (void (*)(void **))free_connection,
                        (int (*)(void *, void *))compare_connection
    );
    connection *server = (connection*)malloc(sizeof(connection));
    int server_fd;
    if(tcp_passive_open(&(server->socket), port_number) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    if(tcp_get_sd(server->socket, &server_fd) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("server %d started\n", server_fd);

    memset(fds, 0, sizeof(fds));
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    server->timestamp = time(NULL);
    dpl_insert_at_index(list, server, 0, false);
}

void insert_client(connection *server, int index) {
    connection *client = (connection*)malloc(sizeof(connection));
    int client_fd;

    if(tcp_wait_for_connection(server->socket, &(client->socket)) != TCP_NO_ERROR) EXIT_FAILURE;
    if(tcp_get_sd(client->socket, &client_fd) != TCP_NO_ERROR) EXIT_FAILURE;
    fds[index].fd = client_fd;
    fds[index].events = POLLIN | POLLRDHUP;
    client->timestamp = time(NULL);
    server->timestamp = time(NULL);
    dpl_insert_at_index(list, client, index, false);
    printf("successfully inserted client: %d\n", client_fd);
}

void read_data(connection *client, int index) {
    int bytes, result;
    sensor_data_t data;

    // read sensor ID
    bytes = sizeof(data.id);
    result = tcp_receive(client->socket,(void *)&data.id,&bytes);
    // read temperature
    bytes = sizeof(data.value);
    result = tcp_receive(client->socket,(void *)&data.value,&bytes);
    // read timestamp
    bytes = sizeof(data.ts);
    result = tcp_receive( client->socket, (void *)&data.ts,&bytes);
    
    client->timestamp = time(NULL);

    //int success;
    if((result == TCP_NO_ERROR) && bytes) {
        //printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value, (long int)data.ts);
        write_data_to_file(data.id, data.value, data.ts);
        printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value, (long int)data.ts);

    }
    
    if(result == TCP_CONNECTION_CLOSED) close_connection(client, index);
}

void close_connection(connection *conn, int index) {
    printf("%s %d has closed connection\n", (index == 0 ? "server" : "client"), fds[index].fd);
    tcp_close(&(conn->socket));
    close(fds[index].fd);
    fds[index].fd = -1;
    list = dpl_remove_at_index(list, index, true);
}

void compress(int old_size) {
    int i, j;
    for(i = 0; i < old_size; i++) {
        if(fds[i].fd == -1) {
            for(j = i; j < old_size-1; j++) {
                fds[j].fd = fds[j+1].fd;
                fds[j+1].fd = -1;
            }
        }
    }
}

void write_data_to_file(sensor_id_t id, sensor_value_t value, sensor_ts_t ts)
{
    fwrite(&id, sizeof(id), 1, fp);
	fwrite(&value, sizeof(value), 1, fp);
 	fwrite(&ts, sizeof(ts), 1, fp);
}


