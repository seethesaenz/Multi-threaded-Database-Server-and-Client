#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include "msg.h"

#define DB "database.db"

int put(struct record *rd){
    //append
    FILE *db_file = fopen(DB, "ab");
    if (db_file == NULL){
        perror("Error opening database");
        return 0;
    }
    fwrite(rd, sizeof(struct record), 1, db_file);
    fclose(db_file);
    return 0;
}

int64_t get(uint32_t id, struct record *rd){
    FILE *db_file = fopen(DB, "rb");
    if (db_file == NULL){
        perror("Error opening database");
        return 0;
    }

    while (fread(rd, sizeof(struct record), 1, db_file) > 0){
        if (rd->id == id){
            fclose(db_file);
            return 1;
        }
    }
    fclose(db_file);
    return 0;
}


void *client_handler(void *arg) {
    int client_socket = *(int *)arg;
    struct msg message;
    printf("connected to a client\n");

    // Receive messages from the client
    while (recv(client_socket, &message, sizeof(message), 0) > 0) {
        // Process the client's request based on the message type
        switch (message.type) {
            case PUT:
                if (put(&message.rd)){
                    message.type = SUCCESS;
                } else {
                    message.type = FAIL;
                }
            case GET:
                if (get(message.rd.id, &message.rd)){
                    message.type = SUCCESS;
                } else {
                    message.type = FAIL;
                }
        }

        // Send a response back to the client
        send(client_socket, &message, sizeof(message), 0);
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;
    if(argc < 2){
        perror("too few arguments");
        return 0;
    }else if(argc > 2){
        perror("too many arguments");
        return 0;
    }
    u_int16_t *port = (uint16_t *) argv[1];

    // Initialize the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        perror("Failed");
        return 0;
    }

    // Set the socket to listening mode
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(*port);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("Failed");
        return 0;
    }
    listen(server_socket, 5);
    // Wait for connection requests from clients
    printf("listening\n");
    while (1) {
        printf("listening\n");
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1){
            perror("Failed to accept");
            continue;
        }

        // Create a new thread to handle the client's requests
        pthread_create(&thread_id, NULL, client_handler, (void *)&client_socket);
    }

    close(server_socket);
    return 0;
}
