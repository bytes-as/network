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
#include <linux/icmp.h>
#include <signal.h>
#include <sys/wait.h>

#define HOST_IP "127.0.0.1"
#define LISTEN_PORT 32164
#define MAX_BUFFER 2048


unsigned int insert_payload(char *buffer) {
  int iphdrlen = sizeof(struct iphdr);
  int udphdrlen = sizeof(struct udphdr);

  char payload[52];
  int j=0;
  while(j<52) {
    payload[j] = 'a' + rand()%26;
    j++;
  }
  strcpy(buffer + iphdrlen + udphdrlen, payload);
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


int main(int argc, char const *argv[]) {
  srand(time(NULL));
  // 1. The program first finds out the IP address corresponding to the given domain name.
  char buffer[MAX_BUFFER];
  int i;
  for(i=0; i<MAX_BUFFER; i++) buffer[i] = '\0';
  // memcpy(buffer, 0, 2048);
  printf("The domain name given input is : %s\n", argv[1]);
  struct hostent* host = gethostbyname(argv[1]);
  strcpy(buffer, inet_ntoa(*(struct in_addr*)(host->h_addr_list[0])));
  printf("IP address of the %s : %s\n", argv[1], buffer);

  // 2. Create the two raw sockets with appropriate parameters and bind them to local IP.
  int rawfd1 = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  int rawfd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  struct sockaddr_in saddr, daddr;
  int saddrlen = sizeof(saddr);
  int daddrlen = sizeof(daddr);

  saddr.sin_family = AF_INET;
  inet_aton(HOST_IP, &saddr.sin_addr);
  saddr.sin_port = htons(LISTEN_PORT);

  daddr.sin_family = AF_INET;
  inet_aton(buffer, &daddr.sin_addr);
  daddr.sin_port = htons(LISTEN_PORT);

  if (rawfd1 < 1) {
    perror("raw socket 2 error : ");
    exit(EXIT_FAILURE);
  }
  if (rawfd2 < 1) {
    perror("raw socket 2 error : ");
    exit(EXIT_FAILURE);
  }

  if(bind(rawfd1, (struct sockaddr *) &saddr, saddrlen) < 0) {
    perror("socket raw 1 bind error: ");
    exit(EXIT_FAILURE);
  }
  if(bind(rawfd2, (struct sockaddr *) &saddr, saddrlen) < 0) {
    perror("socket raw 2 bind error: ");
    exit(EXIT_FAILURE);
  }

  // 3. Set the socket option on S1 to include IPHDR_INCL
  int one = 1;
  const int *val = &one;
  if (setsockopt (rawfd2, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    printf ("Warning: Cannot set HDRINCL!\n");

  // flushing the buffer
  // memcpy(buffer, 0, 2048);
  for(i=0; i<MAX_BUFFER; i++) buffer[i] = '\0';

  unsigned int payload_size = insert_payload(buffer);
  // printf("pay load of size %d included in packet", payload_size);

  unsigned int udpdatagram_size = insert_udpdatagram(buffer, saddr, daddr);
  // printf("udp datagram size : %d", udpdatagram_size);

  unsigned int packet_size =  insert_ipheader(buffer, saddr, daddr, 1); // 1 is the ttl value
  // printf("ip packet size = %d", packet_size);

  struct iphdr ipheader;
  int ipheaderlen = sizeof(struct iphdr);
  int udphdrlen = sizeof(struct udphdr);
  struct icmphdr icmpheader;
  int icmphdrlen = sizeof(struct icmphdr);

  int max_hops = 10;
  int hop_count = 0;
  int count = 0;
  while(hop_count < max_hops){
    count = 0;
    while (1) {
      if(count == 4){
        printf("%d\t*\t*...\n", hop_count);
        val =  &one;
        one = 1;
        if (setsockopt (rawfd2, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0) {
          perror("setsockopt error : ");
          exit(EXIT_FAILURE);
        }
        struct iphdr* new_ipheader = (struct iphdr *)buffer;
        hop_count++;
        new_ipheader->ttl = hop_count;
        break;
      }

      // send the packet
      unsigned int send_bytes = sendto(rawfd1, buffer, packet_size +1, 0,
                                  (struct sockaddr *)&daddr, daddrlen);
      if(send_bytes < 0) {
        exit(1);
      }
      printPacket(buffer, send_bytes);
      // printf("packet of size %d sent\n", send_bytes);
      fd_set rset;
      FD_ZERO(&rset);

      int maxfd = rawfd2+1;
      int select_ret;

      struct timeval timeout;
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      select_ret = select(rawfd2, &rset, NULL, NULL, &timeout);
      if (select_ret < 0){
        perror("select call error | ");
        exit(EXIT_FAILURE);
      } else if (select_ret == 0) {
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        count++;
        printf("timeout...\n");
        continue;
      } else if (FD_ISSET(rawfd2, &rset)) {
        int len = sizeof(daddr);
        memset(buffer, '\0', MAX_BUFFER);
        int msglen = recvfrom(rawfd2, buffer, MAX_BUFFER, 0, (struct sockaddr *) &daddr, &len);
        // if(msglen <= 0) {
        //   i++;
        //   continue;
        // }
        ipheader = *((struct iphdr *) buffer);
        buffer[msglen] = 0;
      }
      if (ipheader.protocol == 1) {
        icmpheader = *((struct icmphdr *) (buffer + ipheaderlen));
        if (icmpheader.type == 3) {
          char ip_address[100];
          // double responste_time = 0.0;
          strcpy(ip_address, inet_ntoa(daddr.sin_addr));
          printf("%d\t%s\t\n", hop_count, ip_address);
          printf("all done.... will kill you\n");
          close(rawfd1);
          close(rawfd2);
          return 0;
        } else if (icmpheader.type == 11) {
          char ip_address[100];
          // double responste_time = 0.0;
          // update the ip address of the receiver
          strcpy(ip_address, inet_ntoa(daddr.sin_addr));
          printf("%d\t%s\t\n", hop_count  , ip_address);
          printf("intermediate... will kill you too\n");
          break;
        }
      }

      hop_count++;
      printf("ttl = %d\n", hop_count);
    }
    // printf("while loop breaks....\n");
    // increase TTL value by 1;
    // hop_count++;
    // printf("hop_count = %d\n", hop_count);
  }
  return 0;
}

//
//
// unsigned int msg_size = insert_payload(buffer);
// // calling to insert udp header
// unsigned int udp_datagram_size = insert_udpdatagram(buffer,saddr, raddr);
// // calling helping function to insert ip address.
// unsigned int ip_packet_len = insert_ipheader(buffer,saddr,raddr, 0);
//
// unsigned int send_bytes;
//  send_bytes = sendto(rawfd, buffer, ip_packet_len,
//    0, (struct sockaddr *) &raddr, raddrlen);
//  if(send_bytes < 0 ){
//    printf("failed...\n");
//  }
