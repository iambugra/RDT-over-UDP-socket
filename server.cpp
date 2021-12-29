#include "lib.h"


int PORT_TROLL = 9319;

struct sockaddr_storage peer_addr;
socklen_t peer_addr_len;

sem_t *mutex, *synch;

Packet outgoing_packets[PACKETS_ARRAY_SIZE];     // both reading and sending routine accesses
Packet incoming_packets[PACKETS_ARRAY_SIZE];

int next_seq_num = WINDOW_SIZE;         
int send_base = WINDOW_SIZE;
int rcv_base = WINDOW_SIZE;

bool sent[PACKETS_ARRAY_SIZE];          // both reading and sending routine accesses
bool acked[PACKETS_ARRAY_SIZE];         // both reading and sending routine accesses
bool avail[PACKETS_ARRAY_SIZE];         // both reading and sending routine accesses
bool received[PACKETS_ARRAY_SIZE];      


Packet make_pkt(bool ACK, bool is_last_chunk, bool isValid, string msg){
    Packet datagram;

    datagram.valid = isValid;
    datagram.isACK = ACK;

    // mutex possible
    datagram.number = next_seq_num;

    datagram.last_chunk = is_last_chunk;
    
    memset(datagram.payload, 0, sizeof(char) * 8);

    int idx = 0;
    for (idx=0; idx<msg.length(); idx++)
        datagram.payload[idx] = msg[idx];

    for (; idx<8; idx++)
        datagram.payload[idx] = '\0';

    datagram.checksum = compute_cheksum(datagram.isACK, datagram.number, datagram.last_chunk, datagram.valid, datagram.payload);

    return datagram;
}


void init(){

    for (int i=0; i<PACKETS_ARRAY_SIZE; i++){
        avail[i] = false;
        acked[i] = false;
        received[i] = false;
        sent[i] = false;
    }

    string dummy_msg1 = "kokaric";
    string dummy_msg2 = "kokorec";
    for (int i=0; i<WINDOW_SIZE; i++){
        outgoing_packets[i] = make_pkt(false, false, false, dummy_msg1);
        incoming_packets[i] = make_pkt(false, false, false, dummy_msg2);
    }
}


vector<string> create_8B_chunks(string buffer){

    vector<string> result;

    int len = buffer.size();                // get lenght of string
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
    buffer.pop_back();                                   // remove new line character

    return create_8B_chunks(buffer);
}


void* reading_routine(void *arg){

    while (1){
        
        vector<string> chunks = read_stdin();
        int size = chunks.size();

        for (int i=0; i<size; i++){
            sem_wait(mutex);

            if (i == size-1) outgoing_packets[next_seq_num] = make_pkt(false, true, true, chunks[i]);
            else outgoing_packets[next_seq_num] = make_pkt(false, false, true, chunks[i]);

            // cout << "after mk_pkt: " << outgoing_packets[next_seq_num].payload << endl;

            avail[next_seq_num] = true;
            sent[next_seq_num] = false;
            // acked[next_seq_num] = false;

            next_seq_num++;
            next_seq_num %= PACKETS_ARRAY_SIZE;

            sem_post(mutex);
        }
    }


    return NULL;
}


void* sending_routine(void *arg){

    sem_wait(synch);

    // struct addrinfo *self = ((sending_routine_params *)arg)->self;
    // char *port_troll = ((sending_routine_params *)arg)->port_troll;
    int sockfd = ((sending_routine_params *)arg)->sockfd;

    // struct sockaddr_in *dest = (struct sockaddr_in *)self->ai_addr;
    // dest->sin_port = PORT_TROLL;

    // cout << dest->sin_port << endl;
    
    while(1) {
        // mutex for send_base maybe
        for (int i=send_base; i<send_base+WINDOW_SIZE; i++){         // iterate within the window and send any packets that can be sent
            i %= PACKETS_ARRAY_SIZE;

            if (sent[i] == false && avail[i] == true) {     // if packet can be sent (in avail list) and not sent before, send it
                
                if (sendto(sockfd, &outgoing_packets[i], sizeof(outgoing_packets[i]), 0, (const struct sockaddr *) &peer_addr, peer_addr_len) == -1){
                    perror("server: pkt sending");
                    cout << strerror(errno) << endl;
                    return NULL;
                }

                // cout << outgoing_packets[i].payload << endl;

                sent[i] = true;
            }

            if (received[i] == true && acked[i] == false) {     // if a packet is rcvd and not acked before, send ack and mark as acked
                
                string ack_msg = "ACK";
                Packet ack = make_pkt(true, true, true, ack_msg);

                // cout << PORT_TROLL << endl;

                if (sendto(sockfd, &ack, sizeof(ack), 0, (const struct sockaddr *) &peer_addr, peer_addr_len) == -1){
                    perror("server: ack sending");
                    return NULL;
                }

                acked[i] = true;

            }
        }

        while (sent[send_base] == true && acked[send_base] == true){        // packet is sent and received correctly, slide send_base to the right
            send_base++;        // mutex maybe
            send_base %= PACKETS_ARRAY_SIZE;                             // for possible wrap up to the beginning
        }


    } 

    return NULL;
}


void extract_data(int idx, bool last){
    
    cout << incoming_packets[idx].payload;
    if (last) cout << endl;

}


void* receiving_routine(void *arg){

    int sockfd = *((int *) arg);
    // struct sockaddr_in troll_addrinfo;
    // struct addrinfo *troll_addrinfo;
    // socklen_t size_trolladdr;
    Packet received_pkt;

    while (1) {
        peer_addr_len = sizeof(peer_addr);
        int len_rcvd = recvfrom(sockfd, &received_pkt, sizeof(Packet), 0, (struct sockaddr *) &peer_addr, &peer_addr_len);
        if (len_rcvd == -1){
            perror("client: pkt rcving");
            return NULL;
        }

        
        // struct sockaddr_in *src = (struct sockaddr_in *)troll_addrinfo->ai_addr;
        
        // PORT_TROLL = htons(troll_addrinfo.sin_port);
        // cout << PORT_TROLL << endl;

        sem_post(synch);

        if ((received_pkt.number - rcv_base) % PACKETS_ARRAY_SIZE < WINDOW_SIZE){

            int calculated_chksum = compute_cheksum(received_pkt.isACK, received_pkt.number, received_pkt.last_chunk, received_pkt.valid, received_pkt.payload);
            
            if (calculated_chksum != received_pkt.checksum){                // cheksums do not match, discard packet
                cout << "checksum mismatch" << endl;
                continue;
            }

            if (received_pkt.isACK){                                        // packet is an ACK packet
                acked[received_pkt.number] = true;
                // cout << "ackkkk" << endl;

            } else {                                                        // it is a data packet
                // acked[received_pkt.number] = false;
                received[received_pkt.number] = true;

                incoming_packets[received_pkt.number] = received_pkt;

            }
        }

        while (received[rcv_base] == true) {
            extract_data(rcv_base, incoming_packets[rcv_base].last_chunk);
            rcv_base++;
            rcv_base %= PACKETS_ARRAY_SIZE;
        }

    }

    return NULL;
}


int main(int argc, char *argv[]){

    if (argc == 1 || argc > 2){
        perror("Port number is not entered or invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }

    char *PORT_SERVER = argv[1];

    int sockfd;
    struct addrinfo hints, *p, *servinfo; 
    int rv;
    struct sockaddr_in cli_addr, troll_addrinfo;

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

    sem_unlink("server_sem_mutex");
    sem_unlink("server_sem_synch");

    mutex = sem_open("server_sem_mutex", O_CREAT, 0600, 1);
    synch = sem_open("server_sem_synch", O_CREAT, 0600, 0);

    pthread_t reading_tid, sending_tid, receiving_tid;

    sending_routine_params send_params;
    send_params.sockfd = sockfd;
    send_params.self = servinfo;
    send_params.port_troll = "-1";

    pthread_create(&reading_tid, NULL, &reading_routine, NULL);
    pthread_create(&sending_tid, NULL, &sending_routine, &send_params);
    pthread_create(&receiving_tid, NULL, &receiving_routine, &sockfd);




    pthread_join(reading_tid, NULL);
    pthread_join(sending_tid, NULL);
    pthread_join(receiving_tid, NULL);
    

    sem_close(mutex);
    sem_close(synch);
    return 0;
}
