#include <iostream>
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

    // if (argc == 1 || argc > 4){
    //     perror("Port number is not entered or invalid number of arguments\n");
    //     exit(EXIT_FAILURE);
    // }

    // PORT_CLIENT = atoi(argv[3]);

    int sockfd;
    struct addrinfo hints, *p, *clientinfo; 
    int rv;
    struct sockaddr_in serv_addr;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo("127.0.0.1", PORT_CLIENT, &hints, &p))) { 
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(clientinfo = p; clientinfo != NULL; clientinfo = clientinfo->ai_next) {
        if ((sockfd = socket(clientinfo->ai_family, clientinfo->ai_socktype, clientinfo->ai_protocol)) == -1) { 
            perror("client: socket"); 
            continue;
        }

        break;
    }

    if (clientinfo == NULL) {
        fprintf(stderr, "client: failed to create socket\n"); 
        return 2;
    }

    if ((bind(sockfd, clientinfo->ai_addr, clientinfo->ai_addrlen) == -1)){
        perror("client: bind"); 
        return 3;
    }

    int len_received_msg, size_cliaddr;

    serv_addr.sin_family    = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(3456);
    
    char *hello = "c: hi";
    sendto(sockfd, (const char *)hello, strlen(hello), 0, (const struct sockaddr *) &serv_addr, sizeof(serv_addr));
    printf("Hello message sent from client.\n");
           
    char buffer[LIMIT_PAYLOAD];
    len_received_msg = recvfrom(sockfd, (char *)buffer, LIMIT_PAYLOAD, MSG_WAITALL, (struct sockaddr *) &serv_addr, (socklen_t *) &size_cliaddr);
    buffer[len_received_msg] = '\0';
    printf("Server : %s\n", buffer);

    
    
    

    return 0;
}
