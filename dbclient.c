#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "msg.h"

int main(int argc, char *argv[]) {
    struct addrinfo hints;
    struct addrinfo *res, *rp;
    int status, sfd;
    struct msg message;

    if (argc < 3){
        perror("Too few arguments");
        return 0;
    }else if (argc > 3){
        perror("Too many arguments");
        return 0;
    }
    char *port = argv[2];
    char *address = argv[1];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    status = getaddrinfo(address, port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "gettaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Initialize the socket
    //client_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);


    // Connect to the server
    //if(connect(client_socket, res->ai_addr, res->ai_addrlen) == -1){
    //    perror("Failed to connect");
    //    return 0;
    //}
    for(rp = res; rp != NULL; rp = rp->ai_next){
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1){
            continue;
        }
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1){
            break;
        }
        close(sfd);
    }
    freeaddrinfo(res);

    if (rp == NULL){
        fprintf(stderr, "could not connect\n");
        exit(EXIT_FAILURE);
    }


    // Send requests to the server
    while (1) {
        // Get user input
        printf("Enter request type (1 to put, 2 to get, 0 to quit): ");
        scanf("%hhu", &message.type);

        if (message.type == PUT) {
            printf("Enter name: ");
            scanf("%s", message.rd.name);
            printf("Enter id: ");
            scanf("%u", &message.rd.id);
        } else if (message.type == GET) {
            printf("Enter id: ");
            scanf("%u", &message.rd.id);
        } else {
            break;
        }

        // Send the request to the server
        send(sfd, &message, sizeof(message), 0);

        // Receive the response from the server
        recv(sfd, &message, sizeof(message), 0);

        // Print the response
        if (message.type == SUCCESS) {
            printf("Success!\n");
            printf("Name: %s\n", message.rd.name);
            printf("Id: %u\n", message.rd.id);
        } else {
            printf("Fail!\n");
        }
    }

    close(sfd);
    return 0;
}
