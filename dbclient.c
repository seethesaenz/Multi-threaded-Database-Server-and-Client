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
    //argument checking
    if (argc < 3){
        perror("Too few arguments");
        return 0;
    }else if (argc > 3){
        perror("Too many arguments");
        return 0;
    }

    char *port = argv[2];
    char *address = argv[1];
    //clearing hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; //return any ipv4 or ipv6
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    status = getaddrinfo(address, port, &hints, &res); //generating addresses
    if (status != 0) {
        fprintf(stderr, "gettaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // trying all address till empty or connects successfully
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
        getchar();

        if (message.type == PUT) {
            printf("Enter name: ");
            fgets(message.rd.name, sizeof(message.rd.name), stdin);
            //finding and removing newline
            message.rd.name[strcspn(message.rd.name, "\n")] = 0;
            printf("Enter id: ");
            scanf("%u", &message.rd.id);
        } else if (message.type == GET) {
            printf("Enter id: ");
            scanf("%u", &message.rd.id);
        } else {
            break;
        }

        // send data to server
        if(write(sfd, &message, sizeof(message)) == -1){
            perror("write failed");
            exit(EXIT_FAILURE);
        }

        // recieve data from server
        if(read(sfd, &message, sizeof(message)) == -1){
            perror("read failed");
            exit(EXIT_FAILURE);
        }

        // Print the response
        if (message.type == SUCCESS) {
            printf("Name: %s\n", message.rd.name);
            printf("Id: %u\n", message.rd.id);
            printf("Success!\n");
        } else {
            printf("Fail!\n");
        }
    }

    close(sfd);
    return 0;
}
