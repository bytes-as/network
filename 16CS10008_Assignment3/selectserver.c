#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>

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

//////////////////////////////////////////////////
////  TCP SOCKET
  // create listening TCP socket
  if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("Cannot create TCP Socket | ");
    exit(0);
  }
  // setting the bits of serrvaddr zero for the fresh input
  // bzero(&servaddr, sizeof(servaddr));
  // binding server addr structure to listenfd
  if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)  {
    perror("TCP SOCKET bind failed\n");
    exit(EXIT_FAILURE);
  }
  listen(listenfd, 10); // limiting the number of clients that can be qeued up.

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

  // clear the descriptor set
  FD_ZERO(&rset);
  // get the maxfd for the n param of the select
  if(listenfd >= udpfd)
    maxdp1 = listenfd + 1;
  else
    maxdp1 = udpfd + 1;

  while(1){
    // SET listenfd AND udpfd IN readset
    FD_SET(listenfd, &rset);
    FD_SET(udpfd, &rset);

    // SET LISTENER FOR TCP SOCKET AND UDP SOCKET IN THE READSET
    nready = select(maxdp1, &rset, NULL, NULL, NULL);

    // for the tcp socket if ready to recieve things
    if(FD_ISSET(listenfd, &rset)) {
      len = sizeof(cliaddr);
      newsockfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
      if(newsockfd < 0){
        perror("Accept Error | ");
        exit(0);
      }
      if((childpid = fork()) == 0)  {
        close(listenfd);
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
        }
        // send a empty string to indicate the end of file to the TCP Socket
        bzero(buffer, sizeof(buffer));
        write(newsockfd, (const char *)buffer, sizeof(buffer)+1);

        // Closing the TCP socket
        close(newsockfd);
        exit(0);
      }
    }

    // SET UP UDP SOCKET TO RECIEVE MESSAGE
    if(FD_ISSET(udpfd, &rset))  {
        len = sizeof(cliaddr);

        bzero(buffer, sizeof(buffer));
        n = recvfrom(udpfd, (char *)buffer, sizeof(buffer), 0,
              (struct sockaddr *)&cliaddr, &len);
        // printf("[UDP-CLIENT] : %s\n", buffer);

        bzero(buffer, sizeof(buffer));
        strcpy(buffer, "Enter the host name to get the server address : ");
        sendto(udpfd, (const char *)buffer, sizeof(buffer), 0,
          (struct sockaddr *)&cliaddr, sizeof(cliaddr));
        printf("[SERVER-UDP] : %s\n", buffer);

        // read incoming host name from the client
        bzero(buffer, sizeof(buffer));
        n = recvfrom(udpfd, (char *)buffer, sizeof(buffer), 0,
              (struct sockaddr *)&cliaddr, &len);
        printf("[UDP-CLIENT] : %s\n", buffer);

        // getting the ip address from the getHostByName() function.
        struct hostent* hostentry = gethostbyname(buffer);

        strcpy(buffer,
          inet_ntoa(* (struct in_addr *)(hostentry->h_addr_list[0])));
        // strcpy(buffer, "Message from client\n");
        // write message to be send to the server
        sendto(udpfd, (const char *)buffer, sizeof(buffer), 0,
          (struct sockaddr*)&cliaddr, sizeof(cliaddr));
        printf("[SERVER-UDP] : %s\n", buffer);
    }
  }

  return 0;
}
