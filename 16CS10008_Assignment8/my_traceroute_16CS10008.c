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
#define LISTEN_IP "0.0.0.0"
#define HOST_IP "0.0.0.0"
#define LISTEN_PORT 32164

#define MAX_PACKET 2048
#define MAX_BUFFER 100
#define MAX_HOP 10

int getDomainIP(char *buffer, const char *argv);
unsigned int insertPayLoad(char *packet);
unsigned int insertUDPHeader(char *packet, struct sockaddr_in saddr, struct sockaddr_in daddr);
unsigned int insertIPHeader(char *packet, struct sockaddr_in saddr, struct sockaddr_in daddr, int ttl);
void printIPHeader(unsigned char *buffer);
void printUDPHeader(unsigned char *buffer);
void printPacket(unsigned char* buffer, unsigned int bytes_sent);

int main(int argc, char const *argv[]) {
  srand(time(NULL));
  char buffer[MAX_BUFFER];
  if (getDomainIP(buffer, argv[1]) != 0) {
    printf("Can't get the domain ip\n");
    exit(EXIT_FAILURE);
  }

  int rawfd1,rawfd2;
	rawfd1 = socket(AF_INET,SOCK_RAW,IPPROTO_RAW);
  if(rawfd1<0)  {
  	perror("Raw udp socket creation failed");
  	return 0;
  }
  rawfd2 = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
 	if(rawfd2<0)  {
    	perror("Raw icmp socket creation failed");
    	return 0;
  }

  int val = 1;
  if(setsockopt(rawfd1,IPPROTO_IP,IP_HDRINCL,&val,sizeof(val))<0)	{
		perror("setsockopt() error");
		return 0;
	}

  struct sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(LISTEN_PORT);
  saddr.sin_addr.s_addr = inet_addr(HOST_IP);
  unsigned int saddrlen = sizeof(saddr);

  struct sockaddr_in daddr;
  daddr.sin_family = AF_INET;
  daddr.sin_port = htons(LISTEN_PORT);
  daddr.sin_addr.s_addr = inet_addr(buffer);
  unsigned int daddrlen = sizeof(daddr);

  if(bind(rawfd2, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
    perror("Bind Error | ");
    exit(EXIT_FAILURE);
  }

  // creating a packet to send
  char packet[MAX_PACKET];
  memset(packet, 0, MAX_PACKET);
  // unsigned int msglen = insertPayLoad(packet);
  // unsigned int udp_datagram_len = insertUDPHeader(packet, saddr, daddr);
  // unsigned int packetlen = insertIPHeader(packet, saddr, daddr);
  // unsigned int send_bytes = sendto(rawfd1, packet, packetlen, 0,
  //   (struct sockaddr *) &daddr, sizeof(daddr));
  // if(send_bytes < 0) {
  //   perror("send error | ");
  //   exit(EXIT_FAILURE);
  // }
  // printPacket(packet,send_bytes);

  struct timeval timeout;
  clock_t start, end;
  double total_time;

  fd_set rset;
  int select_ret;

  int count = 0;
  int hop_count = 0;
  printf("Hop_Count(TTL_Value) \t IP address \t Response_Time\n");
  while (hop_count < MAX_HOP) {
    count = 0;
    while (count < 3) {
      // send the packet
      memset(packet, 0, MAX_PACKET);
      unsigned int msglen = insertPayLoad(packet);
      unsigned int udp_datagram_len = insertUDPHeader(packet, saddr, daddr);
      unsigned int packetlen = insertIPHeader(packet, saddr, daddr, hop_count+1);
      unsigned int send_bytes = sendto(rawfd1, packet, packetlen, 0,
        (struct sockaddr *) &daddr, sizeof(daddr));
      start = clock();
      // printPacket(packet, send_bytes);
      // printf("packet sent\n");
      if(send_bytes < 0) {
        perror("send error | ");
        exit(EXIT_FAILURE);
      }
      // wait on select call
      FD_ZERO(&rset);
      FD_SET(rawfd2, &rset);
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      select_ret = select(rawfd2 + 1, &rset, 0 , 0, &timeout);
      if(select_ret < 0) {
        perror("select call error | ");
        exit(EXIT_FAILURE);
      }
      if(select_ret != 0) {
        if(FD_ISSET(rawfd2, &rset)) {
          memset(packet, 0, MAX_PACKET);
          unsigned int recv_bytes = recvfrom(rawfd2, packet, MAX_PACKET, 0,
            (struct sockaddr *) &daddr, &daddrlen);
          end = clock();
          total_time = ((double)(end-start))/CLOCKS_PER_SEC;
          // printf("some packet received...\n");
        }
        struct iphdr ipheader = *((struct iphdr *) packet);
        if(ipheader.protocol == IPPROTO_ICMP) {
          struct icmphdr icmpheader = *(struct icmphdr *)(packet + sizeof(struct iphdr));
          if(icmpheader.type == 3) {
            printf("%d \t\t\t %s \t %0.3f ms\n", hop_count+1, inet_ntoa(*((struct in_addr *) &ipheader.daddr)), total_time*1000);
            close(rawfd1);
            close(rawfd2);
            return 0;
          }
          if(icmpheader.type == 11) {
            printf("%d \t\t\t %s \t %0.3f ms\n", hop_count+1, inet_ntoa(*((struct in_addr *) &ipheader.daddr)), total_time*1000);
            break;
          }
        }
      }
      count++;
    }
    // if( count >= 3)
    //   printf("%d \t\t\t * \t\t * \n", hop_count+1);
    hop_count++;
  }

  printf("closing application....\n");
  close(rawfd1);
  close(rawfd2);
  return 0;
}

int getDomainIP(char *buffer, const char *argv) {
  memset(buffer, '\0', MAX_BUFFER);
  printf("The domain name given input is : %s\n", argv);
  struct hostent* host = gethostbyname(argv);
  strcpy(buffer, inet_ntoa((struct in_addr)*((struct in_addr*)(host->h_addr_list[0]))));
  printf("IP address of the %s : %s\n", argv, buffer);
  return 0;
}

unsigned int insertPayLoad(char *packet) {
  int i = sizeof(struct iphdr) + sizeof(struct udphdr);
  int j;
  for(j=0; j<52; j++) packet[i+j] = (rand()%26) + 'a';
  // printf("\npayload inserted : %s\nsizeof(payload) = %ld", packet + sizeof(struct iphdr) + sizeof(struct udphdr), strlen(packet + sizeof(struct iphdr) + sizeof(struct udphdr)));
  return sizeof(packet + sizeof(struct udphdr) + sizeof(struct iphdr));
}

unsigned int insertUDPHeader(char *packet, struct sockaddr_in saddr,
  struct sockaddr_in daddr) {
    struct udphdr *udpheader = (struct udphdr *) (packet+sizeof(struct iphdr));
    udpheader->source = saddr.sin_port;
		udpheader->dest = daddr.sin_port;
		udpheader->len = htons(sizeof(struct udphdr) + strlen(packet + sizeof(struct udphdr) + sizeof(struct iphdr)));
		udpheader->check = 0;
    return sizeof(struct udphdr) + strlen(packet + sizeof(struct udphdr) + sizeof(struct iphdr));
}

unsigned int insertIPHeader(char *packet, struct sockaddr_in saddr,
  struct sockaddr_in daddr, int ttl) {
    struct iphdr* ipheader = (struct iphdr *) packet;
    ipheader->ihl = 5;
		ipheader->version = 4;
		ipheader->tos = 0;
		ipheader->id = saddr.sin_port;
		ipheader->ttl = ttl;
		ipheader->protocol = IPPROTO_UDP;
		ipheader->saddr = saddr.sin_addr.s_addr;
		ipheader->daddr = daddr.sin_addr.s_addr;
		ipheader->check = 0;
    ipheader->tot_len = htons(sizeof(struct iphdr)+sizeof(struct udphdr)+strlen(packet + sizeof(struct udphdr) + sizeof(struct iphdr)));
    return sizeof(struct iphdr)+sizeof(struct udphdr)+strlen(packet + sizeof(struct udphdr) + sizeof(struct iphdr));
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
