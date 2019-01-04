// A Simple Client Implementation
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

    // Creating socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8181);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&cliaddr,
            sizeof(cliaddr)) < 0 ) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }


    int n;
    socklen_t len;
    char filename[MAXLINE];
    scanf("%s", filename);
    printf("%s\n", filename);

    // step 1
    // sending the file name
    sendto(sockfd, (const char *)filename, strlen(filename), 0,
			(const struct sockaddr *) &servaddr, sizeof(servaddr));
    //prints the file name that has been sent
    printf("\nFile name \"%s\" sent\n", filename);

    char rec_string[MAXLINE];
    len = sizeof(cliaddr);
    n = recvfrom(sockfd, (char *)rec_string, MAXLINE, 0,
      ( struct sockaddr *) &cliaddr, &len);
    rec_string[n] = '\0';
    if(strcmp(rec_string, "NOTFOUND") == 0)
      printf("%s\n", "File not found");
    // else  if(strcmp(rec_string, "FOUND") == 0){
    //     printf("%s\n", "File found");
    //     close(sockfd);
    // }
    else  {
      char readed_filename[MAXLINE];
      sprintf(readed_filename, "%s_readed", filename);
      FILE *fptr = fopen(readed_filename, "w");
      // printing the first letter of the file read
      // printf("%s\n", rec_string);
      int i=1;
      char requesti[MAXLINE];
      while(1) {
        // char requesti[MAXLINE];
        // to automate the process of requesting word in the file
          sprintf(requesti, "WORD%d", i);
          i++;
        // scanf("%s", requesti);
        // printf("\n%s\n", requesti);
        sendto(sockfd, (const char *)requesti, strlen(requesti), 0,
      			(const struct sockaddr *) &servaddr, sizeof(servaddr));

        len = sizeof(cliaddr);
        n = recvfrom(sockfd, (char *)rec_string, MAXLINE, 0,
          ( struct sockaddr *) &cliaddr, &len);
        rec_string[n] = '\0';
        printf("%s", rec_string);
        if(strcmp(rec_string, "END\n") == 0)
          break;
        fprintf(fptr, "%s", rec_string);
      }// while(strcmp(rec_string, "END\n") != 0);
      // while(1){
      //
      //   sendto(sockfd, (const char *)filename, strlen(filename), 0,
      // 		(const struct sockaddr *) &servaddr, sizeof(servaddr));
      //   len = sizeof(cliaddr);
      //   n = recvfrom(sockfd, (char *)rec_string, MAXLINE, 0,
      //     ( struct sockaddr *) &cliaddr, &len);
      //   rec_string[n] = '\0';
      // }
    }
    // sendto(sockfd, (const char *)filename, strlen(filename), 0,
    //   (const struct sockaddr *) &servaddr, sizeof(servaddr));
    // close(sockfd);
    return 0;
}
