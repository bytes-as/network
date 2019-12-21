#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/ip.h>

#include <unistd.h>

#define LISTEN_IP "127.0.0.1"
#define LISTEN_PORT 32164

int main(int argc, char const *argv[]) {
  int sockfd;
  struct sockaddr_in src_addr;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0) {
    perror("socket error | ");
    exit(1);
  }

  src_addr.sin_family = AF_INET;
  src_addr.sin_port = htons(LISTEN_PORT);
  inet_aton(LISTEN_IP, &src_addr.sin_addr);

  if(bind(sockfd, (const struct sockaddr *)&src_addr, sizeof(src_addr)) < 0){
    perror("bind error | ");
    exit(1);
  }

  char buffer[100];
  int i=0;
  while (i < 7) {
    i++;
    memset(buffer, 0, 100);
    int len = sizeof(src_addr);
    printf("waiting....\n");
    int received_bytes = recvfrom(sockfd, buffer, 100, 0,
      (struct sockaddr *) &src_addr, &len);
    printf("[client]: %s\n", buffer);
    printf("source ip : %s\n", inet_ntoa(src_addr.sin_addr));
    printf("source port : %d\n", ntohs(src_addr.sin_port));
  }
  close(sockfd);
  return 0;
}
