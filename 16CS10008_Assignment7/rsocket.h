// File contain :
// a) #includes for the sockets to work
// b) #define for the timeout T (set to 2 seconds) and
//    the drop probability p
// c) prototypes of all the functions in the API listed
//    above + a function dropMessage()
#define MAXBUFFER 100
#define T 10
#define P 0.1

#ifndef rsocket_h
#define rsocket_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <time.h>
#include <math.h>

#define T 2
#define p 0.1

#define SOCK_MRP 1337

int r_socket(int domain, int type, int protocol);
int r_bind(int sockfd, const struct sockaddr *addr,
   socketlen_t addrlen);
int r_sendto(int sockfd, const void *buf, size_t len, int flags,
   const struct sockaddr *dest_addr, socklen_t addrlen);
int recvfrom(int sockfd, void *buf, size_t len, int flags,
   struct sockaddr *src_addr, socklen_t *addrlen);
int r_close(int fd);

int dropMessage(float p);

#endif
