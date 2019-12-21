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

#define LISTEN_IP "172.217.7.174"
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
    ipheader->tos = 16;
    ipheader->id = raddr.sin_port;
    ipheader->ttl = 225;
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
    memset(buffer, '\0', MAX_BUFFER);
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
    printf("p");
    return send_bytes;
}

int main(int argc, char const *argv[]) {
  char buffer[MAX_BUFFER];
  int i;
  struct iphdr ipheader;
  struct icmphdr icmpheader;
  unsigned int ipheaderlen = sizeof(struct iphdr);
  for(i=0; i<MAX_BUFFER; i++) buffer[i] = '\0';
  // memcpy(buffer, 0, 2048);
  printf("The domain name given input is : %s\n", argv[1]);
  struct hostent* host = gethostbyname(argv[1]);
  strcpy(buffer, inet_ntoa(*(struct in_addr*)(host->h_addr_list[0])));
  printf("IP address of the %s : %s\n", argv[1], buffer);

  // 2. Create the two raw sockets with appropriate parameters and bind them to local IP.
  int rawfd1 = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if (rawfd1 < 1) {
    perror("raw socket 2 error : ");
    exit(EXIT_FAILURE);
  }

  int rawfd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (rawfd2 < 1) {
    perror("raw socket 2 error : ");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in saddr, daddr;
  int saddrlen = sizeof(saddr);
  int daddrlen = sizeof(daddr);

  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = inet_addr(HOST_IP); //INADDR_ANY;
  saddr.sin_port = htons(LISTEN_PORT);
  // configuring the address structure
  daddr.sin_family = AF_INET;
  daddr.sin_addr.s_addr = inet_addr(buffer); //INADDR_ANY;
  daddr.sin_port = htons(LISTEN_PORT);

  // clearing the buffer
  for(i=0; i<MAX_BUFFER; i++) buffer[i] = '\0';

  // if(bind(rawfd1, (struct sockaddr *) &saddr, saddrlen) < 0) {
  //   perror("socket raw 1 bind error: ");
  //   exit(EXIT_FAILURE);
  // }
  if(bind(rawfd2, (struct sockaddr *) &saddr, saddrlen) < 0) {
    perror("socket raw 2 bind error: ");
    exit(EXIT_FAILURE);
  }

  fd_set rset;
  FD_ZERO(&rset);
  int select_ret;

  int count = 0;
  int hop_count = 0;
  while (hop_count < MAX_HOP_COUNT) {
    unsigned int send_bytes = send_random_packet(rawfd1, saddr,
       daddr, hop_count+1, 52);
    if(send_bytes < 0) {
      perror("send error ");
      exit(EXIT_FAILURE);
    }
    // creating the select call to wait on the timeinterva mentioned

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    FD_SET(rawfd2, &rset);
    select_ret = select(rawfd2 + 1, &rset, NULL, NULL, &timeout);
    if(select_ret < 0) {
      perror("select call error");
      exit(EXIT_FAILURE);
    }
    // while (select == 0)
    if(select_ret == 0) {
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      count++;
      if(count == 3) {
        printf("[%d] timeout...\n", hop_count);
        count = 0;
        hop_count++;
      }
      // hop_count++;
      continue;
    } else  {
      // ICMP packet received
      // printf("some ICMP Packet received....!!!!\n");
      if(FD_ISSET(rawfd2, &rset)) {
        icmpheader = *((struct icmphdr *) (buffer + ipheaderlen));
        printf("ipheader.type : %d\n\n", icmpheader.type);
        int len = sizeof(daddr);
        memset(buffer, '\0', MAX_BUFFER);
        unsigned int recv_bytes = recvfrom(rawfd2, buffer, MAX_BUFFER, 0,
          (struct sockaddr *) &daddr, &len);
        if (recv_bytes < 0) {
          perror("recv error : ICMP PACKET ");
          exit(EXIT_FAILURE);
        }
        ipheader = *((struct iphdr *) buffer);
        if (ipheader.protocol == IPPROTO_ICMP) {
          if (icmpheader.type == 3) {
            // print the ip address of the sender and return
            printf("[%d] : %s\n", hop_count+1, inet_ntoa(*((struct in_addr *) &ipheader.daddr)));
            close(rawfd1);
            close(rawfd2);
            return 0;
          } else  {
            if(icmpheader.type == 11) {
              // print the ip address and continue with the reset time out
              // printf("saddr     : %s\n", inet_ntoa(*((struct in_addr *) &ipheader.daddr)));
              printf("[hop count : %d] reached an intermediate hub...", hop_count+1);
              hop_count++;
              continue;
            } else  {
              // set the time interval and wait on a select call again...
            }
          }
        } // if close (ipheader.protocol == 1)
      }
    }
  }
  close(rawfd1);
  // close(rawfd2);
  return 0;
}
