#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <linux/ip.h> /* for ipv4 header */
#include <linux/udp.h> /* for upd header */
#include <linux/icmp.h> // for imcp header
#include <signal.h>
#include <sys/wait.h>

#define LISTEN_IP "127.0.0.1"
#define LISTEN_PORT 32164
#define SERVER_PORT 32164

#define MSG_SIZE 2048
#define MAX_BUFFER 100

#define MAX_HOP 10
#define MAX_TIMEOUT 3

int main(int argc, char const *argv[]) {
  int hop_count, timeout_count, i, j , k;
  int rawfd1, rawfd2;
  int servaddrlen, cliaddrlen;
  struct sockaddr_in servaddr, cliaddr;
  char buffer[MAX_BUFFER];
  struct hostent* host = gethostbyname(argv[1]);
  struct iphdr ipheader;
  struct udphdr udpheader;
  struct icmphdr icmpheader;
  int iphdrlen = sizeof(ipheader);
  int udphdrlen = sizeof(udpheader);
  int icmphdrlen = sizeof(icmpheader);

  printf("The domain name given input is : %s\n", argv[1]);
  strcpy(buffer, inet_ntoa(*(struct in_addr*)(host->h_addr_list[0])));
  printf("IP address of the %s : %s\n", argv[1], buffer);

  rawfd1 = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if (rawfd1 < 0) {
    perror("raw socket 1 error : ");
    exit(EXIT_FAILURE);
  }
  rawfd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (rawfd2 < 0) {
    perror("raw socket 2 error : ");
    exit(EXIT_FAILURE);
  }
  servaddrlen = sizeof(servaddr);
  cliaddrlen = sizeof(cliaddr);

  servaddr.sin_family = AF_INET;
  inet_aton(buffer, &servaddr.sin_addr);
  servaddr.sin_port = htons(SERVER_PORT);

  cliaddr.sin_family = AF_INET;
  inet_aton(LISTEN_IP, &cliaddr.sin_addr);
  servaddr.sin_port = htons(LISTEN_PORT);

  if(bind(rawfd1, (struct sockaddr *) &cliaddr, cliaddrlen) < 0) {
    perror("socket raw 1 bind error: ");
    exit(EXIT_FAILURE);
  }
  if(bind(rawfd2, (struct sockaddr *) &cliaddr, cliaddrlen) < 0) {
    perror("socket raw 2 bind error: ");
    exit(EXIT_FAILURE);
  }

  // 3. Set the socket option on S1 to include IPHDR_INCL
  if(setsockopt(rawfd1, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR | IP_HDRINCL),  &(int){1}, sizeof(int))<0) {
    perror("setsockopt error | ");
    exit(EXIT_FAILURE);
  }

  ipheader.ttl = 1;
  ipheader.ihl = 5;
  ipheader.version = 4;
  ipheader.protocol = 17;
  ipheader.saddr = cliaddr.sin_addr.s_addr;
  ipheader.daddr = servaddr.sin_addr.s_addr;

  hop_count=0;
  while (hop_count<MAX_HOP) {
    // create a udp datagram
    udpheader.source = cliaddr.sin_port;
    udpheader.dest = servaddr.sin_port;
    srand(time(NULL));
    char payload[52];
    i = 0;
    while(i < 52) {
      payload[i] = 'a' + (rand() % 26);
      i++;
    }
    char msg[MSG_SIZE];
    memcpy(msg, &ipheader, iphdrlen);
    memcpy(msg+iphdrlen, &udpheader, udphdrlen);
    memcpy(msg+iphdrlen+udphdrlen, payload, strlen(payload));
    int send_bytes = sendto(rawfd1, msg, strlen(msg)+1, 0, (const struct sockaddr *)&servaddr, servaddrlen);
    if(send_bytes < 0) {
      perror("send error | ");
      exit(EXIT_FAILURE);
    }
    printf("send bytes = %d\n", send_bytes);
    // wait on select call
    fd_set rset;
    FD_ZERO(&rset);
    timeout_count = 0;
    while (timeout_count < MAX_TIMEOUT) {
      /* code */
      FD_SET(rawfd2+1, &rset);
      int maxfd = rawfd2 + 1, select_ret;
      struct timeval timeout;
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      select_ret = select(rawfd2, &rset, NULL, NULL, &timeout);
      if (select_ret < 0) {
        perror("select call error... | ");
        exit(1);
      } else if (select_ret == 0) {
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        printf("timeout | %d...\n", timeout_count);
      } else if (FD_ISSET(rawfd2, &rset)) {
        servaddrlen = sizeof(servaddr);
        memset(msg, '\0', MSG_SIZE);
        int msglen = recvfrom(rawfd2, msg, MSG_SIZE, 0, (struct sockaddr *) &servaddr, &servaddrlen);
        if(msglen <= 0) {
          timeout_count++;
          continue;
        }
        printf("[SERVER] : %s\n", msg);
        ipheader = *((struct iphdr *) msg);
        msg[msglen] = 0;
        if (ipheader.protocol == 1) {
          icmpheader = *((struct icmphdr *) (msg + iphdrlen));
          if (icmpheader.type == 3) {
            printf("%d\t%s\t\n", ipheader.ttl, inet_ntoa(servaddr.sin_addr));
            close(rawfd1);
            close(rawfd2);
            return 0;
          } else if (icmpheader.type == 11) {
            printf("%d\t%s\t\n", ipheader.ttl, inet_ntoa(servaddr.sin_addr));
            ipheader.ttl++;
            hop_count++;
            break;
          }
        }
      }
      timeout_count++;
    }
    hop_count++;
    printf("%d\n", hop_count);
    // increase the value of ttl in ip header
  }
  return 0;
}
