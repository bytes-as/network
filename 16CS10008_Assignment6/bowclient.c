
/*    THE CLIENT PROCESS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <unistd.h>

int main()
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buffer[100];
	// creating TCP Socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	// Filling the server information
	serv_addr.sin_family		= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port = htons(20000);

	// connecting the TCP socket
	printf("trying to connecn...\n");
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
	printf("connecnte\n" );

	// getting the first message from the server
	for(i=0; i < 100; i++) buffer[i] = '\0';
	read(sockfd, (char *)buffer, sizeof(buffer));
  printf("[SERVER-TCP] : %s\n", buffer);

	// recieving the words from the file that server is reading.
  bzero(buffer, sizeof(buffer));
	for(i=0; i < 100; i++) buffer[i] = '\0';
  long int count = 0;
  while(recv(sockfd, buffer, 100, 0)){
    if(!buffer[0])
      break;
    printf("[SERVER-TCP] : %s\n", buffer);
    count++;
  }
  printf("[TCP-CLIENT] : The number of words in the file is : %ld\n", count);
	close(sockfd);
	return 0;
}
