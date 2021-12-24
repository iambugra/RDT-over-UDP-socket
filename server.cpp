#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <pthread.h>
#include <netdb.h>

#include "var.h"


int main(int argc, char *argv[]){

    // if (argc == 1 || argc > 2){
    //     perror("Port number is not entered or invalid number of arguments\n");
    //     exit(EXIT_FAILURE);
    // }

    // PORT_SERVER = atoi(argv[argc-1]);

    int sockfd;
    struct addrinfo hints, *p, *servinfo; 
    int rv;
    struct sockaddr_in cli_addr;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo("127.0.0.1", PORT_SERVER, &hints, &p))) { 
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(servinfo = p; servinfo != NULL; servinfo = servinfo->ai_next) {
        if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) { 
            perror("server: socket"); 
            continue;
        }

        break;
    }

    if (servinfo == NULL) {
        fprintf(stderr, "server: failed to create socket\n"); 
        return 2;
    }

    if ((bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)){
        perror("server: bind"); 
        return 3;
    }

    int size_cliaddr = sizeof(cli_addr);
    char buffer[LIMIT_PAYLOAD];
    int len_received_msg = recvfrom(sockfd, (char *)buffer, LIMIT_PAYLOAD, MSG_WAITALL, (struct sockaddr *) &cli_addr, (socklen_t *) &size_cliaddr);
    buffer[len_received_msg] = '\n';
    printf("Client : %s\n", buffer);

    char *hello = "hi_s";
    sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cli_addr, sizeof(cli_addr));
    printf("Hello message sent.\n");



    return 0;
}
