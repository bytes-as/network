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
#define LISTEN_PORT 20000

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
  printf("sending msg to port: %d\n", ntohs(src_addr.sin_port));
  inet_aton(LISTEN_IP, &src_addr.sin_addr);

  char buffer[100];
  memset(buffer, 0, 100);

  scanf("%[^\n]%*c", buffer);
  int send_bytes = sendto(sockfd, buffer, strlen(buffer)+1, 0, (struct sockaddr *)&src_addr, sizeof(src_addr));
  printf("[client]: (send bytes : %d), %s\n", send_bytes,buffer);  return 0;
  close(sockfd);
  return 0;
}
