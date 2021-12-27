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

#define LIMIT_PAYLOAD 8
#define WINDOW_SIZE 4   
#define NUM_SEQ (2*WINDOW_SIZE)

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
    unsigned short checksum;
    bool isACK;
    int number;
    bool last_chunk;
    string payload;
} Packet;





#endif
