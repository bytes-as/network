// A Simple UDP Server that sends a HELLO message
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    // Create socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family    = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8181);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
            sizeof(servaddr)) < 0 ) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("\nServer Running....\n");

    int n;
    socklen_t len;
    char buffer[MAXLINE];
    char temp[MAXLINE];

    // recieving the file name
    len = sizeof(cliaddr);
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0,
			( struct sockaddr *) &cliaddr, &len);
    buffer[n] = '\0';
    printf("%s\n", buffer);

    FILE *fptr;

    if((fptr = fopen(buffer, "r")) == NULL){
      // if file is not found by the name recieved in buffer
      // this will send a message to client "NOTFOUND"
      sprintf(buffer, "NOTFOUND");
      sendto(sockfd, (const char *)buffer, strlen(buffer), 0,
        (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    } else  {


      sprintf(buffer, "FOUND");
      printf("%s", buffer);
      sendto(sockfd, (const char *)buffer, strlen(buffer), 0,
        (const struct sockaddr *) &cliaddr, sizeof(cliaddr));


      // If file is found then the messages will be read from
      // the file opened
      while(1){
        len = sizeof(cliaddr);
        n = recvfrom(sockfd, (char *)temp, MAXLINE, 0,
    			( struct sockaddr *) &cliaddr, &len);
        temp[n] = '\0';

        if(fscanf(fptr, "%s" , buffer) == 1)
          sendto(sockfd, (const char *)buffer, strlen(buffer), 0,
            (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
        else{
          break;
        }
      }
    }
    // if(fptr = fopen(buffer, "r")){
    //   fscanf(fptr,"%s", buffer);
    //   sendto(sockfd, (const char *)buffer, strlen(buffer), 0,
    //     (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    // } else  {
    //   // if file is not found by the name recieved in buffer
    //   // this will send a message to client "NOTFOUND"
    //   sendto(sockfd, (const char *)error_msg, strlen(error_msg), 0,
    //     (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    // }

    char *buffe = "2";
    sendto(sockfd, (const char *)buffe, strlen(buffer), 0,
      (const struct sockaddr *) &cliaddr, sizeof(cliaddr));

    return 0;
}
