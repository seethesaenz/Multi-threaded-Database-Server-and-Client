#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
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
    while (read(client_socket, &message, sizeof(message)) > 0) {
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
        if (write(client_socket, &message, sizeof(message)) == -1){
            perror("write failed");
            exit(EXIT_FAILURE);
        }
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int server_socket;
    int client_socket;
    struct sockaddr_storage client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;
    struct addrinfo hints;
    struct addrinfo *res;
    int status;

    if (argc != 2){
        fprintf(stderr, "usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(server_socket == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(server_socket, res->ai_addr, res->ai_addrlen) == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if(listen(server_socket, 5) == -1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
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
