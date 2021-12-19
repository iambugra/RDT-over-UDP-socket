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
    struct sockaddr_in servaddr;

    // create UDP IPv4 socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT_SERVER);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    return 0;
}