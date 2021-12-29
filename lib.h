#ifndef __VAR_H__
#define __VAR_H__

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <pthread.h>
#include <netdb.h>
#include <chrono>
#include <vector>
#include <cmath>
#include <semaphore.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <chrono>

#define LIMIT_PAYLOAD 8
#define WINDOW_SIZE 4
#define NUM_SEQ (2*WINDOW_SIZE)
#define BUFFER_SIZE 500
#define PACKETS_ARRAY_SIZE 2048

#define chrono::high_resolution_clock::time_point TimePoint
#define chrono::high_resolution_clock Clock 

using namespace std;

// #define PORT_SERVER "3456"
// #define PORT_CLIENT "7890"

struct handshake_thd_params {
    int sockfd;
    char *msg = "bonjour";
    struct sockaddr_in dest;
    pthread_t parent_tid;
} handshake_thd_params;


typedef struct {
    int sockfd;
    struct addrinfo *self;
    char *port_troll;
} sending_routine_params;


typedef struct {
    int checksum;
    bool isACK;
    int number;
    bool last_chunk;
    bool avail;
    bool sent;
    bool received;
    bool ack_rcvd;
    TimePoint timestamp; // = chrono::high_resolution_clock::now();
    char payload[8];
} Packet;

// chrono::high_resolution_clock::now(); assign this to timestamp   chrono::duration<double, milli>(t_end - t_start).count() gives time diff in milliseconds 



int compute_cheksum(bool isACK, int number, bool last_chunk, bool avail, bool sent, bool received, bool ack_rcvd, char *payload){
    
    int res = 0;

    res += (isACK + number + last_chunk + avail + sent + received + ack_rcvd);

    for (int i=0; i<8; i++)
        res += payload[i];

    return res;
}


Packet make_pkt(int number, bool ACK, bool last, bool avail, bool sent, bool received, bool ack_rcvd, TimePoint timestamp, string msg){
    Packet datagram;

    datagram.avail = avail;
    datagram.isACK = ACK;
    datagram.sent = sent;
    datagram.received = received;
    datagram.ack_rcvd = ack_rcvd;
    datagram.timestamp = timestamp;

    // mutex possible
    datagram.number = number;

    datagram.last_chunk = last;
    
    memset(datagram.payload, 0, sizeof(char) * 8);

    int idx = 0;
    for (idx=0; idx<msg.length(); idx++)
        datagram.payload[idx] = msg[idx];

    for (; idx<8; idx++)
        datagram.payload[idx] = '\0';

    datagram.checksum = compute_cheksum(datagram.isACK, datagram.number, datagram.last_chunk, datagram.avail, datagram.sent, datagram.received, datagram.ack_rcvd, datagram.payload);

    return datagram;
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


#endif
