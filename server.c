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
    char *msg = "s: bugra<3ipek";

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT_SERVER);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        perror("server side bind failed");
        exit(EXIT_FAILURE);
    }

    int len, n;
   
    len = sizeof(cliaddr);
    n = recvfrom(sockfd, (char *)buffer, LIMIT_PAYLOAD, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
   
    buffer[n] = '\0';
   
    printf("Client: %s\n", buffer);

    sendto(sockfd, (const char *)msg, strlen(msg), 0, (const struct sockaddr *) &cliaddr, len);
    printf("Server sent a message.\n"); 


    return 0;
}