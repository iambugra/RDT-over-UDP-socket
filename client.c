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

#include "var.h"

#define NUM_SND_THREAD NUM_SEQ
#define NUM_RCV_THREAD NUM_SEQ



void *receiving_routine(void *args){



    return NULL;
}


void *sending_routine(void *args){


    return NULL;
}


int main(int argc, char *argv[]){

    // if (argc == 1 || argc > 4){
    //     perror("Port number is not entered or invalid number of arguments\n");
    //     exit(EXIT_FAILURE);
    // }

    // PORT_CLIENT = atoi(argv[3]);
    
    int sockfd;
    char buffer[LIMIT_PAYLOAD];
    struct sockaddr_in servaddr, cliaddr;
    char *msg = "bugra <3 ipek";

    // create UDP IPv4 socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // fill sockaddr_in structs for client and server
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT_SERVER);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(PORT_CLIENT);
    cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // bind client's address to socket
    if(bind(sockfd, (const struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0){
        perror("client side bind failed");
        exit(EXIT_FAILURE);
    }



    sendto(sockfd, (const char *)msg, strlen(msg), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

    printf("Client sent a message.\n");

    int size_recv_msg, size_rcv_struct;
    size_recv_msg = recvfrom(sockfd, (char *)buffer, LIMIT_PAYLOAD, MSG_WAITALL, (struct sockaddr *) &servaddr, &size_rcv_struct);
    printf("Server : %s\n", buffer);

    buffer[size_recv_msg] = '\0';

    return 0;
}