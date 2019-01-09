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
    char readed_filename[MAXLINE];
    char *endline = "\n";
    FILE *fptr;
    // Taking input of the file name
    printf("Enter the file name to be read : " );
    scanf("%s", filename);

    // sending the file name
    sendto(sockfd, (const char *)filename, strlen(filename), 0,
			(const struct sockaddr *) &servaddr, sizeof(servaddr));
    //prints the file name that has been sent
    printf("\nClient : File name \"%s\" sent to the server...\n\n", filename);

    // rec_string for storing the recieving message
    char rec_string[MAXLINE];
    len = sizeof(cliaddr);
    // recieving the first message from server.
    n = recvfrom(sockfd, (char *)rec_string, MAXLINE, 0,
      ( struct sockaddr *) &cliaddr, &len);
    rec_string[n] = '\0';
    printf("Server : %s\n\n", rec_string);

    //  If file not found then printing the error message on client side
    if(strcmp(rec_string, "NOTFOUND") == 0)
      printf("%s\n", "File not found");
    else  {
      sprintf(readed_filename, "readed_%s", filename);
      // Creating a file to write the message that are recieved from the server
      fptr = fopen(readed_filename, "w");
      // printing the first letter of the file read
      // printf("%s\n", rec_string);
      int i=0;
      char requesti[MAXLINE];
      while(1) {
        // to automate the process of requesting word in the file
        i++;
        sprintf(requesti, "WORD%d", i);
        printf("Client : %s\n", requesti);
        // Sending the word request
        sendto(sockfd, (const char *)requesti, strlen(requesti), 0,
      			(const struct sockaddr *) &servaddr, sizeof(servaddr));

        len = sizeof(cliaddr);
        n = recvfrom(sockfd, (char *)rec_string, MAXLINE, 0,
          ( struct sockaddr *) &cliaddr, &len);
        rec_string[n] = '\0';
        printf("Server : %s\n", rec_string);
        // printf("%s", rec_string);
        if(strcmp(rec_string, "END\n") == 0){
          fprintf(fptr, "%s", endline);
          fclose(fptr);
          break;
        }
        fprintf(fptr, "%s", rec_string);
      }
      printf("Finished reading file...\n");
    }
    close(sockfd);
    return 0;
}
