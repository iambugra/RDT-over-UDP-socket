#include "lib.h"

using namespace std;


vector<bool> countdown_required;
vector<bool> resend;


void* init_countdown_routine(void *args){

    int sockfd = ((struct handshake_thd_params *)args)->sockfd;
    char *msg = ((struct handshake_thd_params *)args)->msg;
    struct sockaddr_in dest = ((struct handshake_thd_params *)args)->dest;
    pthread_t parent_tid = ((struct handshake_thd_params *)args)->parent_tid;

    auto t_start = chrono::high_resolution_clock::now();
    auto timeout = chrono::high_resolution_clock::now();

    while (1){
        auto t_end = chrono::high_resolution_clock::now();
        if (chrono::duration<double, milli>(t_end - t_start).count() > 1000){
            cout << "1000ms is over send it again" << endl;
            sendto(sockfd, (const char *) msg, strlen(msg), 0, (const struct sockaddr *) &dest, sizeof(dest));

            t_start = chrono::high_resolution_clock::now();
        }
        if (chrono::duration<double, milli>(t_end - timeout).count() > 3000){
            cout << "timeout for cli countdown rtn" << endl;
            pthread_cancel(parent_tid);
            return NULL;
        }
    }


    return NULL;
}


void handshake(int sockfd, struct addrinfo *addrinfo, char *troll_port){

    struct sockaddr_in *dest = (struct sockaddr_in *)addrinfo->ai_addr;
    dest->sin_port = htons(atoi(troll_port));
    // char buffer[15];
    // void *ip_addr = &(((struct sockaddr_in *)addrinfo->ai_addr)->sin_addr);
    // // inet_ntop(addrinfo->ai_family, ip_addr, buffer, sizeof(buffer));
    // unsigned short dest_port = ntohs(dest->sin_port);
    
    // std::cout << dest_port << std::endl;

    char *init_msg = "bonjour";

    if (sendto(sockfd, (const char *) init_msg, strlen(init_msg), 0, (const struct sockaddr *) dest, sizeof(*dest)) == -1){
        // sendto(sockfd, (const char *)hello, strlen(hello), 0, (const struct sockaddr *) &serv_addr, sizeof(serv_addr))
        perror("client: initialize");
        return;
    }

    struct handshake_thd_params params;
    params.sockfd = sockfd;
    params.dest = *dest;
    params.parent_tid = pthread_self();

    pthread_t ptid;
    pthread_create(&ptid, NULL, &init_countdown_routine, &params);

    char buffer[20];
    socklen_t size_trolladdr;
    int len_rcvd = recvfrom(sockfd, (char *) buffer, 20, MSG_WAITALL, (struct sockaddr *) dest, &size_trolladdr);

    fprintf(stderr, "%s\n", buffer);

    pthread_cancel(ptid);


}


void* timer_routine(void *args){

    int seq_num = *((int *)args);

    if (countdown_required[seq_num] == false) return NULL;      //mutex here
    else {
        // countdown baslat
        while (1){
            // if time is up, countdown_required[seq_num] = false, resend[seq_num] = true, return NULL.
            // if countdown_required[seq_num] == false, (ack gelmis), return NULL.
        }
    }


    return NULL;
}


int main(int argc, char *argv[]){

    if (argc == 1 || argc > 4){
        perror("Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }

    char *interface_ip = argv[1];
    char *PORT_TROLL = argv[2];
    char *PORT_CLIENT = argv[3];

    int sockfd;
    struct addrinfo hints, *p, *clientinfo; 
    int rv;
    struct sockaddr_in serv_addr;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(interface_ip, PORT_CLIENT, &hints, &p))) { 
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

    handshake(sockfd, clientinfo, PORT_TROLL);
    
    return 0;
}
