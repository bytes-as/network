#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/ip.h> /* for ipv4 header */
#include <linux/udp.h> /* for upd header */
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#define MSG_SIZE 2048
#define LISTEN_PORT 8080
#define LISTEN_IP "127.0.0.1"
int main(){
 int udpfd;
 struct sockaddr_in saddr_udp;
 int saddr_udp_len;
 char msg[MSG_SIZE] = "Test";
 int msglen;
 int sendlen;
 udpfd = socket(AF_INET, SOCK_DGRAM, 0);
 if(udpfd < 0){
 perror("udp socket");
 exit(__LINE__);
 }

 saddr_udp.sin_family = AF_INET;
 saddr_udp.sin_port = htons(LISTEN_PORT);
 saddr_udp.sin_addr.s_addr = inet_addr(LISTEN_IP);
 saddr_udp_len = sizeof(saddr_udp);
 msglen = strlen(msg);
 sendlen = sendto(udpfd, msg, msglen, 0, (struct sockaddr *)
&saddr_udp, saddr_udp_len);
 if (sendlen < 0)
 perror("sendto");
 else{
 printf("Successfully send\n");
 }
 close(udpfd);
 return 0;
}
