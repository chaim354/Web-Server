/* Generic */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "barrier.h"
/* Network */
#include <netdb.h>
#include <sys/socket.h>

#define BUF_SIZE 100

pthread_t *thread_pool;
pthread_barrier_t barrier;
int threads;
static pthread_mutex_t lock;

char* schedulearg;
char* filename1;
char* filename2;
char *zhost;
char *zport;

// Get host information (used to establishConnection)
struct addrinfo *getHostInfo(char* host, char* port) {

    int r;
    struct addrinfo hints, *getaddrinfo_res;
    // Setup hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((r = getaddrinfo(host, port, &hints, &getaddrinfo_res))) {
        fprintf(stderr, "[getHostInfo:21:getaddrinfo] %s\n", gai_strerror(r));
        return NULL;
    }

    return getaddrinfo_res;
}

// Establish connection with host
int establishConnection(struct addrinfo *info) {
    if (info == NULL) return -1;

    int clientfd;
    for (;info != NULL; info = info->ai_next) {
        if ((clientfd = socket(info->ai_family,
                               info->ai_socktype,
                               info->ai_protocol)) < 0) {
            perror("[establishConnection:35:socket]");
            continue;
        }

        if (connect(clientfd, info->ai_addr, info->ai_addrlen) < 0) {
            close(clientfd);
            perror("[establishConnection:42:connect]");
            continue;
        }

        freeaddrinfo(info);
        return clientfd;
    }
    printf("now");
    freeaddrinfo(info);
    return -1;
}

// Send GET request
void GET(int clientfd, char *host, char *port, char *path) {
    char req[1000] = {0};
    sprintf(req, "GET %s HTTP/1.1\r\nHost: %s:%s\r\n\r\n", path,host,port);
    send(clientfd, req, strlen(req), 0);
}

void  *sThread(void *args){
    int clientfd;
    char buf[BUF_SIZE];
    printf("here");
    while(1){
        clientfd = establishConnection(getHostInfo(zhost, zport));
        printf("no sense");
        if (clientfd == -1) {
            fprintf(stderr,
                    "[main:73] Failed to connect to: %s:%s%s \n",
                    zhost, zport, filename1);
        }

        // Send GET request > stdout
        GET(clientfd, zhost, zport, filename1);
       // pthread_mutex_lock(&lock);
        while (recv(clientfd, buf, BUF_SIZE, 0) > 0) {
            fputs(buf, stdout);
            memset(buf, 0, BUF_SIZE);
        }
      //  pthread_mutex_unlock(&lock);
        printf("why");
        close(clientfd);
        pthread_barrier_wait(&barrier);
    }

}

int main(int argc, char **argv) {

    pthread_mutex_init(&lock,0);

    if (argc != 6) {
        fprintf(stderr, "USAGE: ./httpclient <hostname> <port> <request path>\n");
        return 1;
    }

    zhost = argv[1];
    zport = argv[2];
    threads = atoi(argv[3]);
    schedulearg = argv[4];
    filename1 = argv[5];
    if (argc != 7) {
        filename2 = argv[6];
    }
    thread_pool = (pthread_t*)malloc(sizeof(pthread_t)*threads);
    pthread_barrier_init(&barrier, NULL, threads);
    for(int count = 0; count < threads; count++){
        //pthread_t pthread;
        printf("\n reached creating threads %d\n", count);
        if (pthread_create(&thread_pool[count], NULL, sThread, &count) != 0) {
            printf("hellosdasdads");
            perror("pthread_create() error");
            exit(1);
        }
    }
    while(1){}

    // Establish connection with <hostname>:<port>
    printf("hello");
}
 