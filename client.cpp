#include "lib.h"


vector<bool> countdown_required;
vector<bool> resend;
int next_seq_num = 0;


sem_t *mutex;


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
            pthread_exit(NULL);
            return NULL;
        }
    }

    cout << "countdown rtn exit" << endl;
    return NULL;
}


void* init_helper_routine(void *args){
    int sockfd = ((struct handshake_thd_params *)args)->sockfd;
    struct sockaddr_in dest = ((struct handshake_thd_params *)args)->dest;
    pthread_t parent_tid = ((struct handshake_thd_params *)args)->parent_tid;

    cout << "init helper routine" << endl;

    char buffer[20];
    socklen_t size_trolladdr;
    int len_rcvd = recvfrom(sockfd, (char *) buffer, 20, MSG_WAITALL, (struct sockaddr *) &dest, &size_trolladdr);

    fprintf(stderr, "%s\n", buffer);

    pthread_cancel(parent_tid);
    pthread_exit(NULL);

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

    pthread_t helper_tid, cdown_tid;

    struct handshake_thd_params params1;
    params1.sockfd = sockfd;
    params1.dest = *dest;
    params1.parent_tid = helper_tid;

    pthread_create(&cdown_tid, NULL, &init_countdown_routine, &params1);

    struct handshake_thd_params params2;
    params2.sockfd = sockfd;
    params2.dest = *dest;
    params2.parent_tid = cdown_tid;

    pthread_create(&helper_tid, NULL, &init_helper_routine, &params2);

    
    pthread_join(cdown_tid, NULL);
    cout << "here" << endl;
    pthread_join(helper_tid, NULL);
    


}


vector<string> create_8B_chunks(string buffer){

    vector<string> result;

    int len = buffer.size() - 1;            // get lenght of string, excluding new line character
    int chunk_count = ceil(len/8.);         // how many 8B chunks

    for (int i=0; i<chunk_count; i++){
        string s = buffer.substr(i*8, 8);
        result.push_back(s);
    }

    return result;
}


vector<string> read_stdin(){
    int size = 500;
    char buf[size];

    memset(buf, 0, sizeof(char)*size);                   // clear the buf
    size_t len = read(0, buf, 500);                      // read stdin into buf
    buf[len] = '\0';

    string buffer(buf);

    return create_8B_chunks(buffer);
}


void* sending_routine(void *arg){

    while (1){
        
        vector<string> pkts = read_stdin();
        int size = pkts.size();

        for (int i=0; i<size; i++){
            ;

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

    memset(&hints, 0, sizeof(hints));
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

    sem_unlink("client_sem_mutex");
    mutex = sem_open("client_sem_mutex", O_CREAT, 0600, 1);

    handshake(sockfd, clientinfo, PORT_TROLL);

    cout << "handshake done" << endl;

    
    sem_close(mutex);

    return 0;
}
