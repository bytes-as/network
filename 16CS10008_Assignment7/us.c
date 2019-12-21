#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUFFER 100
#define PORT 8008

int main(int argc, char const *argv[]) {
  int i, sockfd;
  struct sockaddr_in cliaddr, servaddr;
  char *buffer;
  buffer = (char *)malloc(MAXBUFFER * sizeof(buffer));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0)  {
    perror("socket error | ");
    exit(EXIT_FAILURE);
  }

  if(bind(sockfd, ))
  return 0;
}
