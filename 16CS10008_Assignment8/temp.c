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

#define LISTEN_IP "127.0.0.1"
#define LISTEN_PORT 8080
#define MSG_SIZE 2014
#define MAX_BUFFER 100

int main(int argc, char const *argv[]) {
  // 1. The program first finds out the IP address corresponding to the given domain name.
  char buffer[MAX_BUFFER];
  printf("The domain name given input is : %s\n", argv[1]);
  struct hostent* host = gethostbyname(argv[1]);
  strcpy(buffer, inet_ntoa(*(struct in_addr*)(host->h_addr_list[0])));
  printf("IP address of the %s : %s\n", argv[1], buffer);

  // 2. Create the two raw sockets with appropriate parameters and bind them to local IP.
  struct sockaddr_in serv_addr, cli_addr;
  int rawfd1 = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  int rawfd2 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  int serv_addr_len, cli_addr_len;

  serv_addr.sin_family = AF_INET;
  inet_aton(LISTEN_IP, &serv_addr.sin_addr);
  serv_addr.sin_port = htons(32164);
  serv_addr_len = sizeof(serv_addr);

  cli_addr.sin_family = AF_INET;
  inet_aton(buffer, &cli_addr.sin_addr);
  cli_addr.sin_port = htons(LISTEN_PORT);
  cli_addr_len = sizeof(serv_addr);

  if (rawfd1 < 1) {
    perror("raw socket 2 error : ");
    exit(EXIT_FAILURE);
  }
  if (rawfd2 < 1) {
    perror("raw socket 2 error : ");
    exit(EXIT_FAILURE);
  }

  if(bind(rawfd1, (struct sockaddr *) &serv_addr, serv_addr_len) < 0) {
    perror("socket raw 1 bind error: ");
    exit(EXIT_FAILURE);
  }
  if(bind(rawfd2, (struct sockaddr *) &serv_addr, serv_addr_len) < 0) {
    perror("socket raw 2 bind error: ");
    exit(EXIT_FAILURE);
  }

  // 3. Set the socket option on S1 to include IPHDR_INCL
  if(setsockopt(rawfd1, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR | IP_HDRINCL),  &(int){1}, sizeof(int))<0) {
    perror("setsockopt error | ");
    exit(EXIT_FAILURE);
  }

  struct iphdr ipheader;
  struct udphdr udpheader;
  struct icmphdr icmpheader;
  int ipheaderlen = sizeof(ipheaderlen);
  int udpheaderlen = sizeof(udpheaderlen);
  int icmpheaderlen = sizeof(icmpheaderlen);
  int count = 0;
  int max_hops = 10;
  int it = 0, i=0;
  // add socket option once here
  // and put ttl value 1 once
  while(it < max_hops){
    i = 0;
    while (1) {
      if(i == 3){
        // set option in socket
        printf("%d\t*\t*...\n", ipheader.ttl);
        break;
      }
      ipheader.ttl = i+1;
      ipheader.saddr = serv_addr.sin_addr.s_addr;
      ipheader.daddr = cli_addr.sin_addr.s_addr;

      udpheader.source = serv_addr.sin_port;
      udpheader.dest = cli_addr.sin_port;
      // udpheader.source = htons(32164);
      // udpheader.dest = htons(32164);
      srand(time(NULL));
      char payload[52];
      int j=0;
      while(j<52) {
        payload[j] = 'a' + rand()%26;
        j++;
      }
      char msg[MSG_SIZE];
      // c. Append an IP header with the UDP datagram. Set the fields of the IP headers
      // properly, including the TTL field. The destination IP is the IP of your target server
      // (IP corresponding to the domain name you provided).
      memcpy(msg, &ipheader, ipheaderlen);
      memcpy(msg+ipheaderlen, &udpheader, udpheaderlen);
      memcpy(msg+ipheaderlen+udpheaderlen, payload, strlen(payload));
      // d. Send the packet through the raw socket S1.
      sendto(rawfd1, msg, strlen(msg)+1, 0, (const struct sockaddr *)&cli_addr, sizeof(cli_addr));
  // 5. Make a select call to wait for a ICMP message to be received using the raw socket S2 or
  // a timeout value of 1 sec
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
          i++;
          printf("timeout...\n");
          continue;
        } else if (FD_ISSET(rawfd2, &rset)) {
          // check if the target address reached or not
          int len = sizeof(cli_addr);
          memset(msg, '\0', MSG_SIZE);
          int msglen = recvfrom(rawfd2, msg, MSG_SIZE, 0, (struct sockaddr *) &cli_addr, &len);
          if(msglen <= 0) {
            i++;
            continue;
          }
          ipheader = *((struct iphdr *) msg);
          msg[msglen] = 0;
        }
      // 6. If the select() call comes out with receive of an ICMP message in S2:
        // a. If it is a ICMP Destination Unreachable Message (you need to check the IP
        // protocol field to identify a ICMP packet which has a protocol number 1, check
        // https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml for
        // details; then check the ICMP header to check the type field. If type = 3 then it
        // is a ICMP Destination Unreachable Message), then you have reached to the
        // target destination, and received the response from there (to be safe, check that
        // the source IP address of the ICMP Destination Unreachable Message matches
        // with your target server IP address). Print nicely (see format below) and exit after
        // closing the sockets.
        if (ipheader.protocol == 1) {
          icmpheader = *((struct icmphdr *) (msg + ipheaderlen));
          if (icmpheader.type == 3) {
            char ip_address[100];
            // double responste_time = 0.0;
            strcpy(ip_address, inet_ntoa(cli_addr.sin_addr));
            printf("%d\t%s\t\n", it, ip_address);
            close(rawfd1);
            close(rawfd2);
            return 0;
          } else if (icmpheader.type == 11) {
            char ip_address[100];
            // double responste_time = 0.0;
            strcpy(ip_address, inet_ntoa(cli_addr.sin_addr));
            printf("%d\t%s\t\n", it, ip_address);
            break;
          }
        }
        // b. Else if it is an ICMP Time Exceeded Message (check the ICMP header to check
        // the type field. If type = 11 then it is an ICMP Time Exceeded Message), then
        // you have got the response from the Layer 3 device at an intermediate hop
        // specified by the TTL value. Extract the IP address (source IP address of the
        // ICMP Time Exceeded Message) which is the IP address of that hop. Thus, you
        // have identified the device IP at a hop specified by the TTL value. Print nicely
        // (see format below) and then proceed to Step 8.
        // c. Else if you have received any other ICMP packet, it is a spurious packet not
        // related to you. Ignore and go back to wait on the select call (Step 5) with the
        // REMAINING value of timeout.

        // d. For each ICMP Time Exceeded or Destination Unreachable received, measure
        // the response time (time difference between the UDP packet sent, measured just
        // after the sendto() function, and the reception of the ICMP Time Exceeded
        // message or Destination Unreachable, measured after the recvfrom() call).
        // Print it in the following format:
                  // Hop_Count(TTL_Value) IP address Response_Time
        // IP address above is the source IP address of the ICMP Time Exceeded Message
        // or the ICMP Destination Unreachable Message.

      //   7. If the select call times out, repeat from step 4 again. Total number of repeats with the
      // same TTL is 3. If the timeout occurs for all the three UDP packets and you do not
      // receive a ICMP Time Exceeded Message or ICMP Destination Unreachable message in
      // any of them, then print it as follows:
      // Hop_Count(TTL_Value) * *
      // Proceed to the next step if 3 repeats are over.

      // 8. Increment the TTL value by 1 and continue from Step 4 with this new TTL value.
      i++;
      printf("i = %d\n", i);
    }
    printf("while loop breaks....\n");
    // increase TTL value by 1;
    it++;
    printf("it = %d\n", it);
  }
  close(rawfd1);
  close(rawfd2);
  return 0;
}
