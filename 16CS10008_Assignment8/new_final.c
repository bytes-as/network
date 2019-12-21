#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <linux/ip.h> /* for ipv4 header */
#include <linux/udp.h> /* for upd header */

#include <unistd.h>
#include <time.h>
#include <linux/icmp.h>
#include <signal.h>
#include <sys/wait.h>

#define LISTEN_IP "216.58.196.206"
#define HOST_IP "127.0.0.1"
#define LISTEN_PORT 32164
#define DEST_PORT 20000

#define MAX_BUFFER 2048
#define MSGLEN 52

#define MAX_HOP_COUNT 2


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
    ipheader->tos = 0;
    ipheader->id = raddr.sin_port;
    ipheader->ttl = ttl;
    ipheader->protocol = IPPROTO_UDP;
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
    memset(buffer, '\0', MAX_BUFFER);
    for(i=0; i<msglen; i++) {
      buffer[i + iphdrlen + udphdrlen] = (rand() % 26) + 'a';
    }
    buffer[msglen + iphdrlen + udphdrlen] = 0;
    // printf("Enter the message to be send : ");
    // scanf("%[^\n]*c", buffer + iphdrlen + udphdrlen);
    src.sin_family = AF_INET;
    src.sin_addr.s_addr = inet_addr(HOST_IP); //INADDR_ANY;
    src.sin_port = htons(LISTEN_PORT);
    // configuring the address structure
    dest.sin_family = AF_INET;
    // dest.sin_addr.s_addr = inet_addr(buffer); //INADDR_ANY;
    dest.sin_port = htons(LISTEN_PORT);

    unsigned int udpdatagramlen = insert_udpdatagram(buffer, src, dest);
    unsigned int packetlen = insert_ipheader(buffer, src, dest, ttl);
    int one = 1;
    const int *val = &one;
    if (setsockopt (sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
      printf ("Warning: Cannot set HDRINCL!\n");
    unsigned int send_bytes = sendto(sockfd, buffer, packetlen, 0,
       (struct sockaddr *) &dest, sizeof(dest));
    // printPacket(buffer, send_bytes);
    // printf("p");
    return send_bytes;
}

int main(int argc, char const *argv[]) {
  char packet[MAX_BUFFER];
  char msg[MAX_BUFFER] = "fuck off";
  struct sockaddr_in saddr, daddr;

  int rawfd1 = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if (rawfd1 < 0) {
    perror("UDP SOcket error : ");
    exit(EXIT_FAILURE);
  }
  int rawfd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (rawfd2 < 0) {
    perror("UDP SOcket error : ");
    exit(EXIT_FAILURE);
  }

  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port = htons(8000);

  daddr.sin_family = AF_INET;
  // inet_aton("127.0.0.1", &daddr.sin_addr);
  daddr.sin_addr.s_addr = INADDR_ANY;
  daddr.sin_port = htons(32164);
  int one = 1;
  const int *val = &one;
  if (setsockopt (rawfd1, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    printf ("Warning: Cannot set HDRINCL!\n");
  if(bind(rawfd2, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
    perror("socket raw 2 bind error : ");
    exit(EXIT_FAILURE);
  }

  memset(packet, '\0', MAX_BUFFER);
  int iphdrlen = sizeof(struct iphdr);
  int udphdrlen = sizeof(struct udphdr);
  // printf("[Message to be send] : ");
  // scanf("%[^\n]%*c", &packet[iphdrlen + udphdrlen]);

  // int message_len = strlen(packet);

  // struct iphdr* ipheader = (struct iphdr *) packet;
  // ipheader->ihl = 5;
  // ipheader->version = 4;
  // ipheader->tos = 0;
  // ipheader->id = saddr.sin_port;
  // ipheader->ttl = 255;
  // ipheader->protocol = 17;
  // ipheader->saddr = INADDR_ANY;
  // ipheader->daddr = daddr.sin_addr.s_addr;
  // ipheader->check = 0;
  // ipheader->tot_len = sizeof(struct iphdr)+sizeof(struct udphdr)+strlen(msg);
  //
  // struct udphdr* udpheader = (struct udphdr *)(packet + sizeof(struct iphdr));
  // udpheader->source = saddr.sin_port;
  // udpheader->dest = daddr.sin_port;
  // udpheader->check = 0;
  // udpheader->len = htons(8 + strlen(msg));
  //
  // memcpy(packet+iphdrlen+udphdrlen, msg, strlen(msg));
  // // unsigned int send_bytes = send_random_packet(rawfd1, saddr, daddr, 1, 52);
  // unsigned int send_bytes = sendto(rawfd1, packet, ipheader->tot_len + 1, 0,
  //   (struct sockaddr *) &daddr, sizeof(daddr));
  // printf("send bytes : %d\n", send_bytes);

  unsigned int send_bytes = send_random_packet(rawfd1, saddr,
    daddr, 225, 52);
  if(send_bytes < 0) {
    perror("send error ");
    exit(EXIT_FAILURE);
  }


  fd_set rset;
  FD_ZERO(&rset);
  FD_SET(rawfd2, &rset);
  int select_ret;

  struct timeval timeout;
  int count = 0;
  int hop_count=0;
  // while(hop_count < 3){
  //   count = 0;
  //   while(count < 3){
  //     timeout.tv_sec = 1;
  //     timeout.tv_usec = 0;
  //     select_ret = select(rawfd2+1, &rset, NULL, NULL, &timeout);
  //     if(select_ret == 0) {
  //       printf("timeout...\n");
  //       count++;
  //     } else {
  //       if(FD_ISSET(rawfd2, &rset)) {
  //         printf("some packet_received\n");
  //       }
  //     }
  //   }
  //   hop_count++;
  // }
  close(rawfd1);
  close(rawfd2);
  return 0;
}
