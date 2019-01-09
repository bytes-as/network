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
      //sending hello message
      if(fscanf(fptr, "%s", buffer) == 1){
        sendto(sockfd, (const char *)buffer, strlen(buffer), 0,
          (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
      }

      char c; // for the space or endline character tracking
      // Starting word by word reading in while loop
      while(fscanf(fptr, "%s" , buffer) == 1){

        // recieving the request from the client "WORDi"
        len = sizeof(cliaddr);
        n = recvfrom(sockfd, (char *)temp, MAXLINE, 0,
    			( struct sockaddr *) &cliaddr, &len);
        temp[n] = '\0';

        // checking if the word is last of it's line or not
        if((c = fgetc(fptr)) == '\n')
          sprintf(buffer, "%s\n", buffer); // setting the buffer and
          // with end line character
        else
          sprintf(buffer, "%s ", buffer); // setting the buffer with a space

        // sending the message to the client
        sendto(sockfd, (const char *)buffer, strlen(buffer), 0,
          (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
      }

      printf("\nClosing Server...\n");
    }

    return 0;
}
