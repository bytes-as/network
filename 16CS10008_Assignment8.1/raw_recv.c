#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/ip.h>

#include <unistd.h>

// #include <linux/ip.h>
#include <linux/udp.h>

#define LISTEN_IP "127.0.0.1"
#define HOST_IP "127.0.0.1"
#define LISTEN_PORT 32164

#define MAX_BUFFER 2048

void printIPHeader(unsigned char *buffer) {
  struct iphdr *ipheader = (struct iphdr *)buffer;
  printf("\nIP Header consists of the following : \n");
  printf("hl        : %d\n", ipheader->ihl);
  printf("version   : %d\n", ipheader->version);
  printf("ttl       : %d\n", ipheader->ttl);
  printf("protocol  : %d\n", ipheader->protocol);
  printf("tot_len   : %d\n", ipheader->tot_len);
  printf("id        : %d\n", ipheader->id);
  printf("fragoff   : %d\n", ipheader->frag_off);
  printf("check     : %d\n", ipheader->check);
  printf("saddr     : %s\n", inet_ntoa(*((struct in_addr *) &ipheader->saddr)));
  printf("daddr     : %s\n", inet_ntoa(*((struct in_addr *) &ipheader->daddr)));
}

void printUDPHeader(unsigned char *buffer) {
  struct udphdr *udpheader = (struct udphdr *)buffer;
  printf("\nUDP Datagram consists of the following : \n");
  printf("source  : %d\n", ntohs(udpheader->source));
  printf("dest    : %d\n", ntohs(udpheader->dest));
  printf("len     : %d\n", ntohs(udpheader->len));
  printf("check   : %d\n", udpheader->check);
}

void printPayload(unsigned char *buffer) {
  printf("\nMessage send is the following :\n");
  printf("Payload : %s\n", buffer);
}

void printPacket(unsigned char* buffer, unsigned int bytes_sent) {
  unsigned int iphdrlen = sizeof(struct iphdr);
  unsigned int udphdrlen = sizeof(struct udphdr);
  printIPHeader(buffer);
  printUDPHeader(buffer + iphdrlen);
  printPayload(buffer + iphdrlen + udphdrlen);
  printf("Total bytes received : %d\n\n", bytes_sent);
}

int main(int argc, char const *argv[]) {
  int rawfd;
  struct sockaddr_in saddr, raddr;
  int saddrlen = sizeof(saddr);
  int raddrlen = sizeof(raddr);

  char buffer[MAX_BUFFER];
  int recv_bytes;

  struct iphdr ipheader;
  struct udphdr udpheader;

  int iphdrlen = sizeof(struct iphdr);
  int udphdrlen = sizeof(struct udphdr);

  rawfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if (rawfd < 0) {
    perror("socket eroor | ");
    exit(EXIT_FAILURE);
  }

  raddr.sin_family = AF_INET;
  raddr.sin_addr.s_addr = inet_addr(LISTEN_IP); //INADDR_ANY;
  raddr.sin_port = htons(LISTEN_PORT);
  printf("setting up the listener port: %d\n", ntohs(raddr.sin_port));

  if (bind(rawfd, (struct sockaddr *) &raddr, raddrlen) < 0) {
    perror("bind error | ");
    exit(EXIT_FAILURE);
  }
  int i =0 ;
  while (i < 5) {
    i++;
    memset(buffer, 0, MAX_BUFFER);
    saddrlen = sizeof(saddr);
    printf("configuration complete...\nWaiting for message...\n");
    recv_bytes = recvfrom(rawfd, buffer, MAX_BUFFER, 0,
      (struct sockaddr *) &saddr, &saddrlen);
    if(recv_bytes < 0) {
      perror("recv error | ");
      exit(EXIT_FAILURE);
    }
    ipheader = *((struct iphdr *) buffer);
    udpheader = *((struct udphdr *) (buffer+iphdrlen));
    // printf("%d || %d\n", ntohs(udpheader.dest), ntohs(raddr.sin_port));
    // if (udpheader.dest != raddr.sin_port) continue;
    // printf("Packet received\n");
    // printf("hl: %d, version: %d, ttl: %d, protocol: %d\n",
    //   ipheader.ihl, ipheader.version, ipheader.ttl, ipheader.protocol);
    // printf("src: %s : %d\n", inet_ntoa(*((struct in_addr *) &ipheader.saddr)), ntohs(raddr.sin_port));
    // printf("dest: %s : %d\n", inet_ntoa(*((struct in_addr *) &ipheader.daddr)), ntohs(udpheader.dest));
    // printf("payload : %s\n\n\n", buffer + iphdrlen + udphdrlen);
    // struct iphdr newip = ipheader;
    // struct udphdr newudp = udpheader;
    // printf("hl: %d, version: %d, ttl: %d, protocol: %d\n",
    //   newip.ihl, newip.version, newip.ttl, newip.protocol);
    // printf("tot_len : %d, id : %d, fragoff : %d, check : %d\n",
    //   ntohs(newip.tot_len), newip.id, newip.frag_off, newip.check);
    // printf("udp.len : %d\nudp.check : %d\n",
    //  ntohs(newudp.len), newudp.check);
    printPacket(buffer, recv_bytes);
  }
  close(rawfd);
  return 0;
}
