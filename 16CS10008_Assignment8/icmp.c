#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/select.h>

// #define DEST_IP "127.0.0.1"
#define HOST_IP "127.0.0.1"
#define PORT 32164

#define PACKET_SIZE 2048
#define MAX_BUFFER 100

int main(int argc, char const *argv[]) {
  int hop_count, timeout_count, i , j;
  int rawfd1, rawfd2;
  struct sockaddr_in hostaddr, servaddr;
  int hostaddrlen = sizeof(hostaddr);
  int servaddrlen = sizeof(servaddr);
  char buffer[MAX_BUFFER];
  struct hostent* host = gethostbyname(argv[1]);
  struct iphdr ipheader;
  struct udphdr udpheader;
  struct icmphdr icmpheader;
  int iphdrlen = sizeof(ipheader);
  int udphdrlen = sizeof(udpheader);
  int icmphdrlen = sizeof(icmpheader);

  // getting the host ip address from the domain name given as input
  printf("The domain name given input is : %s\n", argv[1]);
  strcpy(buffer, inet_ntoa(*(struct in_addr*)(host->h_addr_list[0])));
  printf("IP address of the %s : %s\n", argv[1], buffer);

  // initializing the sockets
  rawfd1 = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if(rawfd1 < 0) {
    perror("udp socket error...");
    exit(EXIT_FAILURE);
  }
  rawfd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if(rawfd2 < 0) {
    perror("udp socket error...");
    exit(EXIT_FAILURE);
  }
  // configuring the ip address and port number
  servaddr.sin_family = AF_INET;
  inet_aton(buffer, &servaddr.sin_addr);
  servaddr.sin_port = htons(PORT);

  hostaddr.sin_family = AF_INET;
  inet_aton(HOST_IP, &hostaddr.sin_addr);
  hostaddr.sin_port = htons(PORT);

  // binding the socket to the address
  if(bind(rawfd1, (struct sockaddr *) &addr, len) < 0) {
    perror("udp bind error : ");
  }
  if(bind(rawfd2, (struct sockaddr *) &addr, len) < 0) {
    perror("icmp bind error : ");
  }

  // setting up the option for the udp raw socket
  if(setsockopt(rawfd1, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR | IP_HDRINCL),  &(int){1}, sizeof(int))<0) {
    perror("setsockopt error | ");
    exit(EXIT_FAILURE);
  }

  char msg_to_be_send = "AAAAAAA";
  unint8_t data[PACKET_SIZE];
  unsigned int packet_size = build_udp_packet(hostaddr, servaddr, data, ,strlen(msg_to_be_send));

  ipheader.ihl = 5;
  ipheader.version = 4;
  ipheader.ttl = 1;
  ipheader.protocol = 17;
  inet_aton(DEST_IP, &ipheader.saddr);
  inet_aton(HOST_IP, &ipheader.daddr);

  udpheader.source = addr.sin_port;
  udpheader.dset = addr.sin_port;

  char packet[PACKET_SIZE];
  close(rawfd);
  return 0;
}

unsigned int build_udp_packet(struct sockaddr_in src_addr, struct sockaddr_in dst_addr, uint8_t *udp_packet, uint8_t *data, unsigned int data_size)
{
    uint8_t pseudo_packet[MAX_PSEUDO_PKT_SIZE];
    uint16_t length;
    struct udphdr *udph;
    struct pseudo_iphdr *p_iphdr = (struct pseudo_iphdr *)pseudo_packet;

    length = UDP_HDR_SIZE + data_size;
    udph = (struct udphdr *)udp_packet;
    udph->source = src_addr.sin_port;
    udph->dest = dst_addr.sin_port;
    udph->len = htons(length);
    memcpy(udp_packet + UDP_HDR_SIZE, data, data_size);

    if(length + sizeof(struct pseudo_iphdr) > MAX_PSEUDO_PKT_SIZE){
        fprintf(stderr, "Buffer size not enough");
        exit(1);
    }

    // Calculate checksum with pseudo ip header
    p_iphdr->source_addr = src_addr.sin_addr.s_addr;
    p_iphdr->dest_addr = dst_addr.sin_addr.s_addr;
    p_iphdr->zeros = 0;
    p_iphdr->prot = IPPROTO_UDP; //udp
    p_iphdr->length = udph->len;

    // Do NOT use udph->len instead of length.
    // udph->len is in big endian
    memcpy(pseudo_packet + sizeof(struct pseudo_iphdr), udph, length);
    udph->check = checksum(pseudo_packet, sizeof(struct pseudo_iphdr) + length);

    return length;
}
