#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>

#define MAXBUFFER 100
//  SEECT SERVER TO CONNECT DIFFERENT TYPES OF NETWORK METHODS


int main(int argc, char const *argv[]) {
  int       listenfd, newsockfd, udpfd, maxdp1, nready;
  char      buffer[MAXBUFFER];
  pid_t     childpid;
  fd_set    rset;
  ssize_t   n;
  socklen_t len;
  const int on = 1;
  struct    sockaddr_in cliaddr, servaddr;
  char*     message;

  // void sig_child(int);

  // setting up the server details;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(20000);

////////////////////////////////////////////////
////  TCP SOCKET
  // create listening TCP socket
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenfd < 0){
    perror("Cannot create TCP Socket | ");
    exit(0);
  }

  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)	{
    perror("SETSOCKOPT failure\n");
  }

  // setting the bits of serrvaddr zero for the fresh input
  // bzero(&servaddr, sizeof(servaddr));
  // binding server addr structure to listenfd
  if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)  {
    perror("TCP SOCKET bind failed\n");
    exit(EXIT_FAILURE);
  }
  listen(listenfd, 5); // limiting the number of clients that can be qeued up.

  fcntl(listenfd, F_SETFL, O_NONBLOCK);
//////////////////////////////////////////////////////////////////
////  UDP SOCKET
  // create UDP socket
  if((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Cannot create UDP socket\n");
    exit(0);
  }
  // binding server addr structure to udp sockfd
  if( bind(udpfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)  {
    perror("UDP SOCKET bind failed.\n");
    exit(EXIT_FAILURE);
  }

  ////////////////////////////////////////////////////////////////////

  while(1){
    sleep(1);
		printf("Waking up...\n");
    while(1){
			printf("handling tcp connections ... \n");
      len = sizeof(cliaddr);
      errno = 0;
      newsockfd = accept(listenfd, (struct sockaddr*) &cliaddr, &len);
      if(errno == EAGAIN || errno == EWOULDBLOCK){
        printf("breaking again..........................\n");
        break;
      }
      if(newsockfd < 0){
        perror("Accept Error | ");
        break;
      }
      if((childpid = fork()) == 0)  {
        bzero(buffer, sizeof(buffer));
        // writing the message from the server to the tcp-SOCKET
        strcpy(buffer, "Reading file named \'word.txt\'");
        write(newsockfd, (const char *)buffer, sizeof(buffer));
        printf("[SERVER-TCP] : %s\n", buffer);

        // initiating the file pointer for reading the file
        FILE *fptr;
        // validating the file opening
        if((fptr = fopen("word.txt", "r")) == NULL) {
          strcpy(buffer, "NOTFOUND");
          write(newsockfd, (const char *)buffer, sizeof(buffer)+1);
        } else  {
          // reading the words in the file and send to the tcp socket
          while(fscanf(fptr, "%s", buffer) == 1){
            write(newsockfd, (const char *)buffer, sizeof(buffer));
            printf("[SERVER-TCP] : %s\n", buffer);
          }
          // close the file
          fclose(fptr);
          // send a empty string to indicate the end of file to the TCP Socket
          bzero(buffer, sizeof(buffer));
          write(newsockfd, (const char *)buffer, sizeof(buffer)+1);

          // Closing the TCP socket
          close(newsockfd);
        }
      }
    }
    printf("Done handling tcp connections...\n");
    while(1){
      printf("handling udp connections ...  \n");
      len = sizeof(cliaddr);
      bzero(buffer, sizeof(buffer));
      n = recvfrom(udpfd, (char *)buffer, sizeof(buffer), MSG_DONTWAIT,
            (struct sockaddr *)&cliaddr, &len);
      // printf("[UDP-CLIENT] : %s\n", buffer);
      if(n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){
        break;
      }

      bzero(buffer, sizeof(buffer));
      strcpy(buffer, "Enter the host name to get the server address : ");
      sendto(udpfd, (const char *)buffer, sizeof(buffer), MSG_DONTWAIT,
        (struct sockaddr *)&cliaddr, sizeof(cliaddr));
      printf("[SERVER-UDP] : %s\n", buffer);

      // read incoming host name from the client
      bzero(buffer, sizeof(buffer));
      if(fork() == 0){
        n = recvfrom(udpfd, (char *)buffer, sizeof(buffer), 0,
              (struct sockaddr *)&cliaddr, &len);
        printf("[UDP-CLIENT] : %s\n", buffer);

        // getting the ip address from the getHostByName() function.
        struct hostent* hostentry = gethostbyname(buffer);
        if(hostentry){
          strcpy(buffer,
            inet_ntoa(* (struct in_addr *)(hostentry->h_addr_list[0])));
          // strcpy(buffer, "Message from client\n");
          // write message to be send to the server
          sendto(udpfd, (const char *)buffer, sizeof(buffer), 0,
            (struct sockaddr*)&cliaddr, sizeof(cliaddr));
          printf("[SERVER-UDP] : %s\n", buffer);
        }
      }
    }
    printf("done handling udp connections....\nSleeping...\n\n");
  }
  close(listenfd);
  close(udpfd);
  return 0;
}
