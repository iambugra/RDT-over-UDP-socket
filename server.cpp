#include "lib.h"


void handshake(int sockfd, struct addrinfo *troll_addrinfo){

    char buffer[15];
    socklen_t size_trolladdr;
    int len_rcvd = recvfrom(sockfd, (char *)buffer, 15, MSG_WAITALL, (struct sockaddr *) troll_addrinfo, &size_trolladdr);

    fprintf(stderr, "%s\n", buffer);

    char *reply_back = "bonjour a vous aussi";

    sendto(sockfd, (const char *) reply_back, strlen(reply_back), 0, (const struct sockaddr *) troll_addrinfo, sizeof(*troll_addrinfo));

}


int main(int argc, char *argv[]){

    if (argc == 1 || argc > 2){
        perror("Port number is not entered or invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }

    char *PORT_SERVER = argv[1];

    int sockfd;
    struct addrinfo hints, *p, *servinfo, troll_addrinfo; 
    int rv;
    struct sockaddr_in cli_addr;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(NULL, PORT_SERVER, &hints, &p))) { 
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

    handshake(sockfd, &troll_addrinfo);

    unsigned short PORT_TROLL = htons(((struct sockaddr_in *)(troll_addrinfo.ai_addr))->sin_port);
    // unsigned short PORT_TROLL = 49921;

    fprintf(stderr, "%hu\n", PORT_TROLL);

    return 0;
}
