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

#define LIMIT_PAYLOAD 8
#define WINDOW_SIZE 4
#define NUM_SEQ (2*WINDOW_SIZE)
#define BUFFER_SIZE 500
#define PACKETS_ARRAY_SIZE 2048

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
    char payload[8];
} Packet;



int compute_cheksum(bool isACK, int number, bool last_chunk, bool avail, bool sent, bool received, char *payload){
    
    int res = 0;

    res += (isACK + number + last_chunk + avail + sent + received);

    for (int i=0; i<8; i++)
        res += payload[i];

    return res;
}


Packet make_pkt(int number, bool ACK, bool last, bool avail, bool sent, bool received, string msg){
    Packet datagram;

    datagram.avail = avail;
    datagram.isACK = ACK;
    datagram.sent = sent;
    datagram.received = received;

    // mutex possible
    datagram.number = number;

    datagram.last_chunk = last;
    
    memset(datagram.payload, 0, sizeof(char) * 8);

    int idx = 0;
    for (idx=0; idx<msg.length(); idx++)
        datagram.payload[idx] = msg[idx];

    for (; idx<8; idx++)
        datagram.payload[idx] = '\0';

    datagram.checksum = compute_cheksum(datagram.isACK, datagram.number, datagram.last_chunk, datagram.avail, datagram.sent, datagram.received, datagram.payload);

    return datagram;
}


#endif
