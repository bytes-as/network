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

// #define LISTEN_IP "172.217.7.174"
#define LISTEN_IP "127.0.0.1"
#define HOST_IP "127.0.0.1"
#define LISTEN_PORT 32164

#define MAX_BUFFER 2048
#define MAX_MSG 100
#define MAX_HOP 2

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

int getDomainIP(char *buffer, const char *argv) {
  memset(buffer, '\0', MAX_MSG);
  printf("The domain name given input is : %s\n", argv);
  struct hostent* host = gethostbyname(argv);
  strcpy(buffer, inet_ntoa(*(struct in_addr*)(host->h_addr_list[0])));
  printf("IP address of the %s : %s\n", argv, buffer);
  return 0;
}

int main(int argc, char const *argv[]) {
  unsigned char buffer[MAX_MSG];
  unsigned char packet[MAX_BUFFER];
  memset(buffer, '\0', MAX_MSG);
  // printf("The domain name given input is : %s\n", argv[1]);
  // struct hostent* host = gethostbyname(argv[1]);
  // strcpy(buffer, inet_ntoa(*(struct in_addr*)(host->h_addr_list[0])));
  // printf("IP address of the %s : %s\n", argv[1], buffer);
  if (getDomainIP(buffer, argv[1]) != 0) {
    printf("Can't get the domain ip\n");
    exit(EXIT_FAILURE);
  }
  printf("\nTraceroute for %s :\n", buffer);
  struct sockaddr_in saddr, daddr;
  // declaring address
  int saddrlen = sizeof(saddr);
  int daddrlen = sizeof(daddr);
  // declaring headers
  struct iphdr ipheader;
  struct udphdr udpheader;
  // declaring header lengths
  int iphdrlen = sizeof(struct iphdr);
  int udphdrlen = sizeof(struct udphdr);
  int icmphdrlen = sizeof(struct icmphdr);

  int rawfd1;
  rawfd1 = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if(rawfd1 < 0) {
    perror("socket error | ");
    exit(EXIT_FAILURE);
  }

  int rawfd2;
  rawfd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if(rawfd2 < 0) {
    perror("socket error | ");
    exit(EXIT_FAILURE);
  }
  // configuring the address structure
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = inet_addr(HOST_IP); //INADDR_ANY;
  saddr.sin_port = htons(LISTEN_PORT);
  // configuring the address structure
  daddr.sin_family = AF_INET;
  daddr.sin_addr.s_addr = inet_addr(buffer); //INADDR_ANY;
  daddr.sin_port = htons(LISTEN_PORT);

  // calling to insert data packet
  unsigned int msg_size = insert_payload(packet);
  // calling to insert udp header
  unsigned int udp_datagram_size = insert_udpdatagram(packet, saddr, daddr);
  // calling helping function to insert ip address.
  unsigned int ip_packet_len = insert_ipheader(packet, saddr, daddr, 0);

  if(bind(rawfd2, (struct sockaddr *) &saddr, saddrlen) < 0) {
    perror("socket raw 2 bind error: ");
    exit(EXIT_FAILURE);
  }

  int one = 1;
  const int *val = &one;
  if (setsockopt (rawfd1, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    printf ("Warning: Cannot set HDRINCL!\n");

  unsigned int send_bytes = sendto(rawfd1, packet, ip_packet_len,
    0, (struct sockaddr *) &daddr, daddrlen);
  if(send_bytes < 0 ){
    printf("failed...\n");
  }
  // printPacket(packet, send_bytes);
  printf("packet sent\n");
  // unsigned int send_bytes = send_random_packet(rawfd1, &saddr, &daddr, 0, 52);

  int count = 0;
  struct timeval timeout;
  fd_set rset;
  FD_ZERO(&rset);
  FD_SET(rawfd2, &rset);
  int select_ret;

  clock_t start, end;
  double total_time;
  printf("Hop_Count(TTL_Value) \t IP address \t Response_Time\n");
  int hop_count = 0;
  while (hop_count < MAX_HOP) {
    count = 0;
    while(count < 3) {
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      ip_packet_len = insert_ipheader(packet, saddr, daddr, hop_count+1);
      send_bytes = sendto(rawfd1, packet, ip_packet_len, 0,
         (struct sockaddr *) &daddr, daddrlen);
      if(send_bytes < 0) {
        printf("[%d] send error\n", hop_count+1);
        exit(EXIT_FAILURE);
      }
      printf("packet sent\n");
      // printPacket(packet, send_bytes);
      start = clock();
      if(send_bytes < 0) {
        perror("send error ");
        exit(EXIT_FAILURE);
      }
      // creating the select call to wait on the timeinterva mentioned
      FD_ZERO(&rset);
      FD_SET(rawfd2, &rset);
      select_ret = select(rawfd2 + 1, &rset, NULL, NULL, &timeout);
      if(select_ret < 0) {
        perror("select call error");
        exit(EXIT_FAILURE);
      }
      if (select_ret == 0) {
        // printf("count = %d\n", count);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        count++;
        if(count == 3) {
          // printf("[%d] timeout...\n", hop_count + 1);
          printf("%d \t\t\t * \t\t * \n", hop_count+1);
          hop_count++;
          count = 0;
          break;
        }
      } else {
        if(FD_ISSET(rawfd2, &rset)) {
          int len = sizeof(daddr);
          memset(buffer, '\0', MAX_BUFFER);
          unsigned int recv_bytes = recvfrom(rawfd2, buffer, MAX_BUFFER,
            0, (struct sockaddr *) &daddr, &len);
          end = clock();
          total_time = ((double)(end-start)/CLOCKS_PER_SEC);
          if (recv_bytes < 0) {
            perror("recv error : ICMP Packet ");
            exit(EXIT_FAILURE);
          }
          struct iphdr ipheader;
          struct icmphdr icmpheader;
          unsigned int ipheaderlen = sizeof(struct iphdr);
          ipheader = *((struct iphdr *) buffer);
          if(ipheader.protocol == IPPROTO_ICMP) {
            icmpheader = *((struct icmphdr *)(buffer + ipheaderlen));
            // printf("ipheader.type = %d\n", icmpheader.type);
            if (icmpheader.type == 3) {
              printf("%d \t\t\t %s \t %0.3f ms\n", hop_count+1, inet_ntoa(*((struct in_addr *) &ipheader.daddr)), total_time*1000);
              close(rawfd1);
              close(rawfd2);
              return 0;
            }
            if (icmpheader.type == 11) {
              printf("[hop count : %d] reached an intermediate hub...", hop_count+1);
              hop_count++;
              count = 0;
              break;
            }
          }
        }
      }
    }
  }
  close(rawfd1);
  close(rawfd2);
  return 0;
}
