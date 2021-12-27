#include "lib.h"



int next_seq_num = WINDOW_SIZE;         
Packet packets[PACKETS_ARRAY_SIZE];     // both reading and sending routine accesses
int send_base = WINDOW_SIZE;

bool sent[PACKETS_ARRAY_SIZE];          // both reading and sending routine accesses
bool acked[PACKETS_ARRAY_SIZE];         // both reading and sending routine accesses
bool avail[PACKETS_ARRAY_SIZE];         // both reading and sending routine accesses



sem_t *mutex;


int compute_cheksum(bool isACK, int number, bool last_chunk, bool valid, char *payload){
    
    int res = 0;

    res += (isACK + number + last_chunk + valid);

    for (int i=0; i<8; i++)
        res += payload[i];

    return res;
}


Packet make_pkt(bool ACK, bool is_last_chunk, bool isValid, string msg){
    Packet datagram;

    datagram.valid = isValid;

    datagram.isACK = ACK;

    // mutex possible
    datagram.number = next_seq_num;

    datagram.last_chunk = is_last_chunk;
    
    memset(datagram.payload, 0, sizeof(char) * 8);

    int idx = 0;
    for (idx=0; idx<msg.length()-1; idx++)
        datagram.payload[idx] = msg[idx];

    for (; idx<8; idx++)
        datagram.payload[idx] = '\0';

    datagram.checksum = compute_cheksum(datagram.isACK, datagram.number, datagram.last_chunk, datagram.valid, datagram.payload);

    return datagram;
}


void init(){

    for (int i=0; i<PACKETS_ARRAY_SIZE; i++)
        avail[i] = false;

    string dummy_msg = "kokaric";
    for (int i=0; i<WINDOW_SIZE; i++)
        packets[i] = make_pkt(false, false, false, dummy_msg);
    



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
    char buf[BUFFER_SIZE];

    memset(buf, 0, sizeof(char)*BUFFER_SIZE);            // clear the buf
    size_t len = read(0, buf, BUFFER_SIZE);              // read stdin into buf
    buf[len] = '\0';

    string buffer(buf);

    return create_8B_chunks(buffer);
}


void* reading_routine(void *arg){

    while (1){
        
        vector<string> chunks = read_stdin();
        int size = chunks.size();

        for (int i=0; i<size; i++){
            sem_wait(mutex);

            if (i == size-1) packets[next_seq_num] = make_pkt(true, true, true, chunks[i]);
            else packets[next_seq_num] = make_pkt(true, false, true, chunks[i]);

            avail[next_seq_num] = true;
            sent[next_seq_num] = true;
            acked[next_seq_num] = false;

            next_seq_num++;
            next_seq_num %= PACKETS_ARRAY_SIZE;

            sem_post(mutex);
        }
    }


    return NULL;
}


void* sending_routine(void *arg){

    struct addrinfo *self = ((sending_routine_params *)arg)->self;
    char *port_troll = ((sending_routine_params *)arg)->port_troll;
    int sockfd = ((sending_routine_params *)arg)->sockfd;

    struct sockaddr_in *dest = (struct sockaddr_in *)self->ai_addr;
    dest->sin_port = htons(atoi(port_troll));
    
    while(1) {
        
        // mutex for send_base maybe
        for(int i=send_base; i<send_base+WINDOW_SIZE; i++){         // iterate within the window and send any packets that can be sent
            i %= PACKETS_ARRAY_SIZE;

            if (sent[i] == false && avail[i] == true) {
                // send, make sent true
                if (sendto(sockfd, (const char *) &packets[i], sizeof(packets[i]), 0, (const struct sockaddr *) dest, sizeof(*dest)) == -1){
                    perror("client: sending rtn");
                    return NULL;
                }

                sent[i] = true;
            }
        }

        if (sent[send_base] == true && acked[send_base] == true){        // packet is sent and received correctly, slide send_base to the right
            send_base++;        // mutex maybe
            send_base %= PACKETS_ARRAY_SIZE;
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

    for (clientinfo = p; clientinfo != NULL; clientinfo = clientinfo->ai_next) {
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

    init();

    sem_unlink("client_sem_mutex");
    mutex = sem_open("client_sem_mutex", O_CREAT, 0600, 1);


    pthread_t reading_tid, sending_tid;

    sending_routine_params params;
    params.sockfd = sockfd;
    params.self = clientinfo;
    params.port_troll = PORT_TROLL;

    pthread_create(&reading_tid, NULL, &reading_routine, NULL);
    pthread_create(&sending_tid, NULL, &sending_routine, &params);


    

    
    pthread_join(reading_tid, NULL);
    pthread_join(sending_tid, NULL);
    

    sem_close(mutex);

    return 0;
}
