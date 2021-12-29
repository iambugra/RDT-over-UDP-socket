#include "lib.h"


struct sockaddr_storage peer_addr;
socklen_t peer_addr_len;


Packet outgoing_packets[PACKETS_ARRAY_SIZE];     // both reading and sending routine accesses
Packet incoming_packets[PACKETS_ARRAY_SIZE];     // only receiving

int next_seq_num = WINDOW_SIZE;              // reading  
int send_base = WINDOW_SIZE;                 // sending
int rcv_base = WINDOW_SIZE;                  // receiving


void send_ack(int sockfd, int number){

    string msg = "ACK";
    Packet ack = make_pkt(number, true, true, true, false, false, false, msg);

    if (sendto(sockfd, &ack, sizeof(ack), 0, (const struct sockaddr *) &peer_addr, peer_addr_len) == -1){
        perror("server: ack sending");
        return;
    }
    ack.sent = true;
    cout << "ack sent " << ack.number << endl;

}


void init(){

    string dummy_msg1 = "kokaric";
    string dummy_msg2 = "kokorec";
    for (int i=0; i<WINDOW_SIZE; i++){
        outgoing_packets[i] = make_pkt(0, false, false, false, false, false, false, dummy_msg1);
        incoming_packets[i] = make_pkt(0, false, false, false, false, false, false, dummy_msg2);
    }
}


void* reading_routine(void *arg){

    while (1){
        
        vector<string> chunks = read_stdin();
        int size = chunks.size();

        for (int i=0; i<size; i++){

            if (i == size-1) outgoing_packets[next_seq_num] = make_pkt(next_seq_num, false, true, true, false, false, false, chunks[i]);
            else outgoing_packets[next_seq_num] = make_pkt(next_seq_num, false, false, true, false, false, false, chunks[i]);

            next_seq_num++;
            next_seq_num %= PACKETS_ARRAY_SIZE;
        }
    }

    return NULL;
}


void* sending_routine(void *arg){

    int sockfd = ((sending_routine_params *)arg)->sockfd;
    
    while(1) {
        // mutex for send_base maybe
        for (int i=send_base; i<send_base+WINDOW_SIZE; i++){         // iterate within the window and send any packets that can be sent
            i %= PACKETS_ARRAY_SIZE;

            if (outgoing_packets[i].sent == false && outgoing_packets[i].avail == true) {     // if packet can be sent (available, read) and not sent before, send it
                
                if (sendto(sockfd, &outgoing_packets[i], sizeof(outgoing_packets[i]), 0, (const struct sockaddr *) &peer_addr, peer_addr_len) == -1){
                    perror("server: pkt sending");
                    cout << strerror(errno) << endl;
                    return NULL;
                }

                outgoing_packets[i].sent = true;
            }

            while (outgoing_packets[send_base].sent == true && outgoing_packets[send_base].ack_rcvd == true){        // packet is sent and received correctly, slide send_base to the right
                send_base++;        // mutex maybe
                send_base %= PACKETS_ARRAY_SIZE;                             // for possible wrap up to the beginning
            }

        }

    } 

    return NULL;
}


void extract_data(int idx, bool last){


    cout  << "RECEIVED: ";
    cout << "seq#: " << incoming_packets[idx].number;
    cout << " payload: " << incoming_packets[idx].payload;

    // cout << incoming_packets[idx].payload;
    if (last) cout << endl;

}


void* receiving_routine(void *arg){

    int sockfd = *((int *) arg);
    Packet received_pkt;

    bool ack_sent[PACKETS_ARRAY_SIZE];
    for (int i=0; i<PACKETS_ARRAY_SIZE; i++)
        ack_sent[i] = false;

    while (1) {
        peer_addr_len = sizeof(peer_addr);
        int len_rcvd = recvfrom(sockfd, &received_pkt, sizeof(Packet), 0, (struct sockaddr *) &peer_addr, &peer_addr_len);
        if (len_rcvd == -1){
            perror("client: pkt rcving");
            return NULL;
        }

        int calculated_chksum = compute_cheksum(received_pkt.isACK, received_pkt.number, received_pkt.last_chunk, received_pkt.avail, received_pkt.sent, received_pkt.received, received_pkt.ack_rcvd, received_pkt.payload);
        if (calculated_chksum != received_pkt.checksum){                // cheksums do not match, discard packet
            cout << "checksum mismatch" << endl;
            continue;
        }

        if (received_pkt.isACK) {
            outgoing_packets[received_pkt.number].ack_rcvd = true;
            cout << " ack rcvd " << received_pkt.number << endl;
            
        } else {

            if ((received_pkt.number - rcv_base) % PACKETS_ARRAY_SIZE < WINDOW_SIZE) {

                incoming_packets[received_pkt.number] = received_pkt;
                incoming_packets[received_pkt.number].received = true;
                
                if (ack_sent[received_pkt.number] == false) {

                    send_ack(sockfd, received_pkt.number);
                    ack_sent[received_pkt.number] = true;
                }
            }
        }

        while (incoming_packets[rcv_base].received == true) {
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
    
    return 0;
}
