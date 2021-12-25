#ifndef __VAR_H__
#define __VAR_H__

#define LIMIT_PAYLOAD 8
#define WINDOW_SIZE 4   
#define NUM_SEQ (2*WINDOW_SIZE)

// #define PORT_SERVER "3456"
// #define PORT_CLIENT "7890"

struct init_thd_params {
    int sockfd;
    char *msg = "bonjour";
    struct sockaddr_in dest;
    pthread_t parent_tid;
} init_thd_params;

#endif
