#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "var.h"

int main(){
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

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT_SERVER);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(PORT_CLIENT);
    cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

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