/*
Name : Arun Singh
Roll NO. : 16CS10008

'-' -> cons & '+' -> pros

Signal driven I/O :
- Signal handling may lead to a non-reliable functionality of the program
  as the signal can be interrupt.
- Interruts are slower thus if a program have many interrupts based
  executions then the whold program will gonna slow down.

+ low processing require as the process is not doing anythign until it
  receives some I/O interruption.

Non-Blocking I/O (with flags):
- as it needs to check all the I/O request one by one even if there is no I/O
  interruption
- from the perspective of developing the code, it might be a bit chaotic to do
  all the things in single go but still better than signal handling as there we
  can't get the source of the interrupt.

+ It ensures the reliabile functionality of handling I/O requests.

Judging by the above notions, In my opinion I think if there is enough resources to have the non-blocking using flags
then one should go for the Non-Blocking using flag, but otherwise the signal handling might be a good option ignoring
the extra efforts to manage the code for handling signal handling.
*/

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
  // printf("sending msg to port: %d\n", ntohs(src_addr.sin_port));
  inet_aton(LISTEN_IP, &src_addr.sin_addr);

  char buffer[100];
  memset(buffer, 0, 100);
  printf("Enter the message to be send : ");
  scanf("%[^\n]%*c", buffer);
  int send_bytes = sendto(sockfd, buffer, strlen(buffer)+1, 0, (struct sockaddr *)&src_addr, sizeof(src_addr));
  printf("[CLIENT]: %s, (send bytes : %d)\n", buffer, send_bytes);

  memset(buffer, 0, 100);
  int len = sizeof(src_addr);
  int recv_bytes = recvfrom(sockfd, buffer, 100, 0, (struct sockaddr *) &src_addr, &len);
  printf("received bytes : %d\n", recv_bytes);
  printf("[SERVER] : %s\n", buffer);
  close(sockfd);
  return 0;
}
