#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/ip.h>
// #include <linux/ip.h>

#include <unistd.h>

#include <linux/udp.h>

// #define LISTEN_IP "172.217.7.174"
#define LISTEN_IP "127.0.0.1"
#define HOST_IP "127.0.0.1"
#define LISTEN_PORT 20000

#define MAX_BUFFER 2048
#define MAX_MSG 100

unsigned int insert_payload(char *buffer) {
  int iphdrlen = sizeof(struct iphdr);
  int udphdrlen = sizeof(struct udphdr);
  printf("[Message to be send] : ");
  scanf("%[^\n]%*c", &buffer[iphdrlen + udphdrlen]);
  return strlen(&buffer[iphdrlen + udphdrlen]);
}

unsigned int insert_udpdatagram(unsigned char *buffer,
  struct sockaddr_in saddr, struct sockaddr_in daddr) {
     int iphdrlen = sizeof(struct iphdr);
     int udphdrlen = sizeof(struct udphdr);
     unsigned int msg_size = strlen(buffer + iphdrlen + udphdrlen);
     struct udphdr* udpheader = (struct udphdr *)(buffer + iphdrlen);
     udpheader->source = saddr.sin_port;
     udpheader->dest = daddr.sin_port;
     udpheader->check = 0;
     udpheader->len = htons(
       sizeof(struct udphdr) + strlen(buffer + iphdrlen + udphdrlen));
    return ntohs(udpheader->len);
}

unsigned int insert_ipheader(char *buffer,
  struct sockaddr_in saddr, struct sockaddr_in raddr, int ttl) {
    int iphdrlen = sizeof(struct iphdr);
    int udphdrlen = sizeof(struct udphdr);
    unsigned int udp_datagram_size =
      udphdrlen + strlen(buffer + iphdrlen + udphdrlen);
    struct iphdr* ipheader = (struct iphdr *) buffer;
    ipheader->ihl = 5;
    ipheader->version = 4;
    ipheader->tos = 16;
    ipheader->id = raddr.sin_port;
    ipheader->ttl = ttl;
    ipheader->protocol = 17;
    ipheader->saddr = saddr.sin_addr.s_addr;
    ipheader->daddr = raddr.sin_addr.s_addr;
    ipheader->check = 0;
    ipheader->tot_len = htons(sizeof(struct iphdr) + udp_datagram_size);
    return ntohs(ipheader->tot_len);
}

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
  printf("Total bytes sent : %d\n\n", bytes_sent);
}

unsigned int send_random_packet(int sockfd, struct sockaddr_in src,
  struct sockaddr_in dest, int ttl, int msglen) {
    unsigned char buffer[MAX_BUFFER];
    int udphdrlen = sizeof(struct udphdr);
    int iphdrlen = sizeof(struct iphdr);
    // unsigned int msglen = insert_payload(buffer);
    srand(time(NULL));
    int i = 0;
    for(i=0; i<msglen; i++) {
      buffer[i + iphdrlen + udphdrlen] = (rand() % 26) + 'a';
    }
    buffer[msglen + iphdrlen + udphdrlen] = 0;
    // printf("Enter the message to be send : ");
    // scanf("%[^\n]*c", buffer + iphdrlen + udphdrlen);
    unsigned int udpdatagramlen = insert_udpdatagram(buffer, src, dest);
    unsigned int packetlen = insert_ipheader(buffer, src, dest, ttl);
    int one = 1;
    const int *val = &one;
    if (setsockopt (sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
      printf ("Warning: Cannot set HDRINCL!\n");
    unsigned int send_bytes = sendto(sockfd, buffer, packetlen, 0,
       (struct sockaddr *) &dest, sizeof(dest));
    printPacket(buffer, send_bytes);
    return send_bytes;
}

int main(int argc, char const *argv[]) {
  unsigned char buffer[MAX_BUFFER];
  int rawfd;
  struct sockaddr_in saddr, raddr;
  // declaring address
  int saddrlen = sizeof(saddr);
  int raddrlen = sizeof(raddr);
  // declaring headers
  struct iphdr ipheader;
  struct udphdr udpheader;
  // declaring header lengths
  int iphdrlen = sizeof(struct iphdr);
  int udphdrlen = sizeof(struct udphdr);

  rawfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if(rawfd < 0) {
    perror("socket error | ");
    exit(EXIT_FAILURE);
  }
  // configuring the address structure
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = inet_addr(HOST_IP); //INADDR_ANY;
  saddr.sin_port = htons(LISTEN_PORT);
  // configuring the address structure
  raddr.sin_family = AF_INET;
  raddr.sin_addr.s_addr = inet_addr(LISTEN_IP); //INADDR_ANY;
  raddr.sin_port = htons(LISTEN_PORT);
  //
  // // calling to insert data packet
  // unsigned int msg_size = insert_payload(buffer);
  // // calling to insert udp header
  // unsigned int udp_datagram_size = insert_udpdatagram(buffer,saddr, raddr);
  // // calling helping function to insert ip address.
  // unsigned int ip_packet_len = insert_ipheader(buffer,saddr,raddr, 0);
  //
  // {				/* lets do it the ugly way.. */
  //     int one = 1;
  //     const int *val = &one;
  //     if (setsockopt (rawfd, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
  //       printf ("Warning: Cannot set HDRINCL!\n");
  //   }
  // unsigned int send_bytes;
  // send_bytes = sendto(rawfd, buffer, ip_packet_len,
  //   0, (struct sockaddr *) &raddr, raddrlen);
  // if(send_bytes < 0 ){
  //   printf("failed...\n");
  // }
  // printPacket(buffer, send_bytes);
  unsigned int send_bytes = send_random_packet(rawfd, saddr, raddr, 0, 52);
  return 0;
}
