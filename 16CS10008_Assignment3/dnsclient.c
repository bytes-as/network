// UDP client program
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define PORT 20000
#define MAXLINE 100
int main()
{
    int sockfd;
    char buffer[MAXLINE];
    char* message = "Hello Server";
    struct sockaddr_in servaddr;

    int n, len;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket creation failed");
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bzero(buffer, sizeof(buffer));
    strcpy(buffer, "HELLO");
    sendto(sockfd, (const char*)buffer, strlen(buffer),
           0, (const struct sockaddr*)&servaddr,
           sizeof(servaddr));

    bzero(buffer, sizeof(buffer));
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0,
          (struct sockaddr *)&servaddr, &len);
    printf("[SERVER-UDP] : %s\n", buffer);

    bzero(buffer, sizeof(buffer));
    printf("[UDP-CLIENT] : ");
    scanf("%s", buffer);
    sendto(sockfd, (const char *)buffer, strlen(buffer), 0,
      (const struct sockaddr *)&servaddr, sizeof(servaddr));
    // printf("%s\n", buffer);

    // receive server's response
    bzero(buffer, sizeof(buffer));
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0,
          (struct sockaddr *)&servaddr, &len);
    printf("[SERVER-UDP] : %s\n", buffer);

    close(sockfd);
    return 0;
}
